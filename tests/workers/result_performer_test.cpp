//
// Created by Roman Yakimkin on 30.06.2026.
//

#include <gtest/gtest.h>
#include <sstream>
#include <thread>
#include <chrono>
#include "workers/result_performer.h"
#include "core/models/factorized_number.h"

// Вспомогательный класс для перехвата stdout
class CoutCapture {
    std::stringstream buffer;
    std::streambuf* old_cout;
public:
    CoutCapture() {
        old_cout = std::cout.rdbuf(buffer.rdbuf());
    }

    ~CoutCapture() {
        std::cout.rdbuf(old_cout);
    }

    std::string str() {
        return buffer.str();
    }
};

TEST(ResultPerformerTest, ProcessSingleElement) {
    ConcurrentQueue<model::FactorizedNumber> queue;
    workers::ResultPerformer performer(&queue);

    model::FactorizedNumber result;
    result.m_numb = 100;
    result.m_factors = {2, 2, 5, 5};
    result.m_thread_numb = 1;
    result.m_duration_mks = std::chrono::microseconds(150);

    queue.push(result);
    queue.close();

    CoutCapture capture;
    performer();

    std::string output = capture.str();

    EXPECT_TRUE(output.find("Thread #1") != std::string::npos);
    EXPECT_TRUE(output.find("Number : 100") != std::string::npos);
    EXPECT_TRUE(output.find("Factors : 2, 2, 5, 5") != std::string::npos);
    EXPECT_TRUE(output.find("150mks") != std::string::npos);
}

TEST(ResultPerformerTest, ProcessMultipleElements) {
    ConcurrentQueue<model::FactorizedNumber> queue;
    workers::ResultPerformer performer(&queue);

    for (int i = 1; i <= 5; i++) {
        model::FactorizedNumber result;
        result.m_numb = i * 10;
        result.m_factors = {static_cast<uint64_t>(i), static_cast<uint64_t>(10)};
        result.m_thread_numb = i;
        result.m_duration_mks = std::chrono::microseconds(i * 100);
        queue.push(result);
    }
    queue.close();

    CoutCapture capture;
    performer();

    std::string output = capture.str();

    EXPECT_TRUE(output.find("Number : 10") != std::string::npos);
    EXPECT_TRUE(output.find("Number : 20") != std::string::npos);
    EXPECT_TRUE(output.find("Number : 30") != std::string::npos);
    EXPECT_TRUE(output.find("Number : 40") != std::string::npos);
    EXPECT_TRUE(output.find("Number : 50") != std::string::npos);
    EXPECT_TRUE(output.find("Thread #1") != std::string::npos);
    EXPECT_TRUE(output.find("Thread #2") != std::string::npos);
    EXPECT_TRUE(output.find("Thread #3") != std::string::npos);
    EXPECT_TRUE(output.find("Thread #4") != std::string::npos);
    EXPECT_TRUE(output.find("Thread #5") != std::string::npos);
}

TEST(ResultPerformerTest, StopsWhenQueueClosed) {
    ConcurrentQueue<model::FactorizedNumber> queue;
    workers::ResultPerformer performer(&queue);
    std::atomic performer_stopped{false};

    std::thread performer_thread([&performer, &performer_stopped]() {
        performer();
        performer_stopped = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    queue.close();

    performer_thread.join();

    EXPECT_TRUE(performer_stopped);
}

TEST(ResultPerformerTest, MultiThreadedProcessing) {
    ConcurrentQueue<model::FactorizedNumber> queue;
    workers::ResultPerformer performer(&queue);

    const int CNT_ITEMS = 10;

    // Producer поток
    std::thread producer([&queue]() {
        for (int i = 0; i < CNT_ITEMS; ++i) {
            model::FactorizedNumber result;

            result.m_numb = (i + 1) * 7;
            result.m_factors = {static_cast<uint64_t>(i + 1), 7};
            result.m_thread_numb = 1;
            result.m_duration_mks = std::chrono::microseconds(i * 50);

            queue.push(result);
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Небольшая задержка
        }
        queue.close();
    });

    // Performer поток
    CoutCapture capture;
    std::thread performer_thread([&performer]() {
        performer();
    });

    producer.join();
    performer_thread.join();

    std::string output = capture.str();

    // Проверяем, что все элементы обработаны
    for (int i = 0; i < CNT_ITEMS; ++i) {
        std::string expected_number = "Number : " + std::to_string((i + 1) * 7);
        EXPECT_TRUE(output.find(expected_number) != std::string::npos)
            << "Expected to find: " << expected_number;
    }
}

TEST(ResultPerformerTest, ProcessPrimeNumber) {
    ConcurrentQueue<model::FactorizedNumber> queue;
    workers::ResultPerformer performer(&queue);

    model::FactorizedNumber result;

    result.m_numb = 17;      // Простое число
    result.m_factors = {17}; // Только само число
    result.m_thread_numb = 1;
    result.m_duration_mks = std::chrono::microseconds(50);

    queue.push(result);
    queue.close();

    CoutCapture capture;
    performer();

    std::string output = capture.str();

    EXPECT_TRUE(output.find("Number : 17") != std::string::npos);
    EXPECT_TRUE(output.find("Factors : 17") != std::string::npos);
}

TEST(ResultPerformerTest, ProcessManyFactors) {
    ConcurrentQueue<model::FactorizedNumber> queue;
    workers::ResultPerformer performer(&queue);

    model::FactorizedNumber result;

    result.m_numb = 1024; // 2^10
    result.m_factors = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
    result.m_thread_numb = 1;
    result.m_duration_mks = std::chrono::microseconds(200);

    queue.push(result);
    queue.close();

    CoutCapture capture;
    performer();

    std::string output = capture.str();

    EXPECT_TRUE(output.find("Number : 1024") != std::string::npos);
    EXPECT_TRUE(output.find("2, 2, 2, 2, 2, 2, 2, 2, 2, 2") != std::string::npos);
}