#include <cmath>
#include <iostream>
#include <thread>
#include <csignal>

#include "helpers/cores.h"
#include "infrastructure/app_config.h"
#include "workers/defs.h"
#include "workers/factorizer.h"
#include "workers/prime_numbers_provider.h"
#include "workers/result_performer.h"
#include "workers/udp_receiver.h"

std::atomic<bool> g_stop_requested{false};

void signalHandler(int) { g_stop_requested.store(true); }

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << std::unitbuf;
    std::cout << "Starting factorization server...\n";

    try {
        infra::AppConfig cfg;

        workers::PrimeNumbersGeneratedNotifier pmg_notifier;

        // создать очереди
        ConcurrentQueue<uint64_t> number_queue(100000);
        ConcurrentQueue<model::FactorizedNumber> result_queue;

        // создать классы для потоков
        workers::UDPReceiver receiver(cfg.host(), cfg.port(), &number_queue, cfg.min_val(), cfg.max_val());

        auto max_prime_number = static_cast<uint64_t>(sqrt(static_cast<double>(cfg.max_val()) + 1));
        workers::PrimeNumbersProvider prime_numbers_provider(max_prime_number, pmg_notifier);

        auto threads = helpers::factorizationThreads();
        std::vector<workers::Factorizer> factorizers;
        factorizers.reserve(threads);
        for (size_t i = 0; i < threads; ++i) {
            factorizers.emplace_back(i + 1, &number_queue, &result_queue, &prime_numbers_provider, std::ref(pmg_notifier));
        }

        workers::ResultPerformer result_performer(&result_queue);

        // запустить потоки
        std::jthread t_receiver( std::move(receiver));
        std::jthread t_prime_numbers_provider(std::ref(prime_numbers_provider));

        std::vector<std::jthread> t_factorizers;
        t_factorizers.reserve(factorizers.size());
        for (auto& factorizer : factorizers) t_factorizers.emplace_back(std::ref(factorizer));

        std::jthread t_result_performer(std::ref(result_performer));

        while (!g_stop_requested.load()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::cout << "\nGraceful shutdown initiated...\n";

        t_receiver.request_stop();

        number_queue.close();
        result_queue.close();

        std::cout << "All threads stopped successfully.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "UNKNOWN ERROR" << std::endl;
        return 1;
    }

    return 0;
}
