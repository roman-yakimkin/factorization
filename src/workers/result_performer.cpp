//
// Created by Roman Yakimkin on 26.06.2026.
//

#include <iostream>

#include "result_performer.h"

#include <sstream>

namespace workers {
    ResultPerformer::ResultPerformer(ResultQueue *queue) : m_queue(queue) {}

    std::string ResultPerformer::join(const std::vector<uint64_t>& vec) {
        std::ostringstream oss;

        for (size_t i = 0; i < vec.size(); i++) {
            if (i > 0) oss << ", ";
            oss << vec[i];
        }

        return oss.str();
    }

    void ResultPerformer::operator()() {
        std::cout << "Result performer thread started.\n";

        try {
            while (auto factorized = m_queue->pop()) {
                std::cout << "Thread #" << factorized.value().m_thread_numb << "\n"
                          << "Number : " << factorized.value().m_numb << "\n"
                          << "Factors : " << join(factorized.value().m_factors) << "\n"
                          << "Duration : " << factorized.value().m_duration_mks.count() << "mks. \n";
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Result performer Fatal error " << e.what() << "\n";
        }
        catch (...) {
            std::cerr << "[Thread] Unknown fatal error.\n";
        }

        std::cout << "Result performer thread stopped gracefully.\n";
    }
} // workers