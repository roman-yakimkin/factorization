//
// Created by Roman Yakimkin on 01.07.2026.
//

#include <gtest/gtest.h>
#include "workers/udp_receiver.h"
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Вспомогательный класс для отправки UDP-пакетов
class UdpSender {
    int sock;
    sockaddr_in dest_addr;
public:
    UdpSender(const std::string& host, uint16_t port) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) throw std::runtime_error("Sender socket error");

        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &dest_addr.sin_addr);
    }

    ~UdpSender() {
        if (sock >= 0) close(sock);
    }

    void sendNumber(uint64_t number) {
        // Конвертируем в network byte order
        uint64_t net_value = htobe64(number);
        sendto(sock, &net_value, sizeof(net_value), 0,
               reinterpret_cast<sockaddr*>(&dest_addr), sizeof(dest_addr));
    }

    void sendRawData(const void* data, size_t size) {
        sendto(sock, data, size, 0,
               reinterpret_cast<sockaddr*>(&dest_addr), sizeof(dest_addr));
    }
};

TEST(UdpReceiverTest, ReceiveSingleNumber) {
    ConcurrentQueue<uint64_t> queue;
    const uint16_t port = 50001;

    workers::UDPReceiver receiver("127.0.0.1", port, &queue, 0, 1000000);

    std::jthread receiver_thread([&receiver](std::stop_token stoken) {
        receiver(stoken);
    });

    // Даём время receiver запуститься
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Отправляем число
    UdpSender sender("127.0.0.1", port);
    sender.sendNumber(12345);

    // Ждём обработки
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Останавливаем receiver
    receiver_thread.request_stop();
    receiver_thread.join();

    // Проверяем результат
    auto result = queue.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 12345);
}

TEST(UdpReceiverTest, ReceiveMultipleNumbers) {
    ConcurrentQueue<uint64_t> queue;
    const uint16_t port = 50002;

    workers::UDPReceiver receiver("127.0.0.1", port, &queue, 0, 1000000);

    std::jthread receiver_thread([&receiver](std::stop_token stoken) {
        receiver(stoken);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    UdpSender sender("127.0.0.1", port);

    // Отправляем несколько чисел
    std::vector<uint64_t> sent_numbers = {100, 200, 300, 400, 500};
    for (uint64_t num : sent_numbers) {
        sender.sendNumber(num);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    receiver_thread.request_stop();
    receiver_thread.join();
    queue.close();

    // Проверяем, что все числа получены
    std::vector<uint64_t> received;
    while (true) {
        auto val = queue.pop();
        if (!val.has_value()) break;
        received.push_back(val.value());
    }

    ASSERT_EQ(received.size(), sent_numbers.size());
    for (size_t i = 0; i < sent_numbers.size(); ++i) {
        EXPECT_EQ(received[i], sent_numbers[i]);
    }
}

TEST(UdpReceiverTest, FilterByMinValue) {
    ConcurrentQueue<uint64_t> queue;
    const uint16_t port = 50003;

    workers::UDPReceiver receiver("127.0.0.1", port, &queue, 100, 1000000);

    std::jthread receiver_thread([&receiver](std::stop_token stoken) {
        receiver(stoken);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    UdpSender sender("127.0.0.1", port);

    // Отправляем числа: одно меньше min, одно в диапазоне
    sender.sendNumber(50);   // Должно быть отфильтровано (< 100)
    sender.sendNumber(150);  // Должно пройти (>= 100)

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    receiver_thread.request_stop();
    receiver_thread.join();
    queue.close();

    auto result = queue.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 150);

    // Второго элемента быть не должно
    auto second = queue.pop();
    EXPECT_FALSE(second.has_value());
}

TEST(UdpReceiverTest, FilterByMaxValue) {
    ConcurrentQueue<uint64_t> queue;
    const uint16_t port = 50004;

    workers::UDPReceiver receiver("127.0.0.1", port, &queue, 0, 1000);

    std::jthread receiver_thread([&receiver](std::stop_token stoken) {
        receiver(stoken);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    UdpSender sender("127.0.0.1", port);

    // Отправляем числа: одно в диапазоне, одно больше max
    sender.sendNumber(500);   // Должно пройти (< 1000)
    sender.sendNumber(1500);  // Должно быть отфильтровано (>= 1000)

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    receiver_thread.request_stop();
    receiver_thread.join();
    queue.close();

    auto result = queue.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 500);

    auto second = queue.pop();
    EXPECT_FALSE(second.has_value());
}

TEST(UdpReceiverTest, IgnoreInvalidPacketSize) {
    ConcurrentQueue<uint64_t> queue;
    const uint16_t port = 50005;

    workers::UDPReceiver receiver("127.0.0.1", port, &queue, 0, 1000000);

    std::jthread receiver_thread([&receiver](std::stop_token stoken) {
        receiver(stoken);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    UdpSender sender("127.0.0.1", port);

    // Отправляем пакет неправильного размера (4 байта вместо 8)
    uint32_t invalid_data = 12345;
    sender.sendRawData(&invalid_data, sizeof(invalid_data));

    // Отправляем корректный пакет
    sender.sendNumber(999);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    receiver_thread.request_stop();
    receiver_thread.join();
    queue.close();

    // Должен быть только корректный пакет
    auto result = queue.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 999);

    auto second = queue.pop();
    EXPECT_FALSE(second.has_value());
}

TEST(UdpReceiverTest, StopWithStopToken) {
    ConcurrentQueue<uint64_t> queue;
    const uint16_t port = 50006;

    workers::UDPReceiver receiver("127.0.0.1", port, &queue, 0, 1000000);

    std::atomic<bool> receiver_stopped{false};

    std::jthread receiver_thread([&receiver, &receiver_stopped](std::stop_token stoken) {
        receiver(stoken);
        receiver_stopped = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Запрашиваем остановку
    receiver_thread.request_stop();
    receiver_thread.join();
    queue.close();

    EXPECT_TRUE(receiver_stopped);
}

TEST(UdpReceiverTest, ReceiveLargeNumbers) {
    ConcurrentQueue<uint64_t> queue;
    const uint16_t port = 50007;

    workers::UDPReceiver receiver("127.0.0.1", port, &queue, 0, UINT64_MAX);

    std::jthread receiver_thread([&receiver](std::stop_token stoken) {
        receiver(stoken);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    UdpSender sender("127.0.0.1", port);

    // Отправляем большие числа
    std::vector<uint64_t> large_numbers = {
        1000000000000ULL,
        9999999999999ULL,
        UINT64_MAX - 1
    };

    for (uint64_t num : large_numbers) {
        sender.sendNumber(num);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    receiver_thread.request_stop();
    receiver_thread.join();
    queue.close();

    std::vector<uint64_t> received;
    while (true) {
        auto val = queue.pop();
        if (!val.has_value()) break;
        received.push_back(val.value());
    }

    ASSERT_EQ(received.size(), large_numbers.size());
    for (size_t i = 0; i < large_numbers.size(); ++i) {
        EXPECT_EQ(received[i], large_numbers[i]);
    }
}

TEST(UdpReceiverTest, BindToAnyAddress) {
    ConcurrentQueue<uint64_t> queue;
    const uint16_t port = 50008;

    workers::UDPReceiver receiver("0.0.0.0", port, &queue, 0, 1000000);

    std::jthread receiver_thread([&receiver](std::stop_token stoken) {
        receiver(stoken);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    UdpSender sender("127.0.0.1", port);
    sender.sendNumber(777);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    receiver_thread.request_stop();
    receiver_thread.join();
    queue.close();

    auto result = queue.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 777);
}