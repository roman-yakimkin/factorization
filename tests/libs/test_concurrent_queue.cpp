//
// Created by Roman Yakimkin on 30.06.2026.
//

#include <gtest/gtest.h>
#include "libs/concurrent_queue.h"
#include <thread>
#include <chrono>
#include <vector>

TEST(ConcurrentQueueTest, BasicPushPop) {
    ConcurrentQueue<int> queue;

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);

    EXPECT_TRUE(queue.push(10));
    EXPECT_TRUE(queue.push(20));
    EXPECT_TRUE(queue.push(30));

    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 3);

    auto val1 = queue.pop();
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(val1.value(), 10);

    auto val2 = queue.pop();
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(val2.value(), 20);

    auto val3 = queue.pop();
    ASSERT_TRUE(val3.has_value());
    EXPECT_EQ(val3.value(), 30);

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST(ConcurrentQueueTest, MaxSizeLimit) {
    ConcurrentQueue<int> queue(2);

    EXPECT_TRUE(queue.push(1));
    EXPECT_TRUE(queue.push(2));
    EXPECT_EQ(queue.size(), 2);
}

TEST(ConcurrentQueueTest, CloseQueue) {
    ConcurrentQueue<int> queue;

    queue.push(100);
    queue.push(200);

    queue.close();

    EXPECT_FALSE(queue.push(300));

    auto val1 = queue.pop();
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(val1.value(), 100);

    auto val2 = queue.pop();
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(val2.value(), 200);

    auto val3= queue.pop();
    EXPECT_FALSE(val3.has_value());
}

TEST(ConcurrentQueueTest, ProducerConsumer) {
    ConcurrentQueue<int> queue;
    const int CNT_ITEMS = 1000;
    std::vector<int> results;

    std::thread producer([&queue]() {
        for (int i = 0; i < CNT_ITEMS; i++) queue.push(i);

        queue.close();
    });

    std::thread consumer([&queue, &results]() {
        for (int i = 0; i < CNT_ITEMS; i++) {
            auto val = queue.pop();
            if (!val.has_value()) break;

            results.push_back(val.value());
        }

        queue.close();
    });

    producer.join();
    consumer.join();

    ASSERT_EQ(results.size(), CNT_ITEMS);
    for (int i = 0; i < CNT_ITEMS; i++) {
        EXPECT_EQ(results[i], i);
    }
}

TEST(ConcurrentQueueTest, BlockingPush) {
    ConcurrentQueue<int> queue(2);

    queue.push(1);
    queue.push(2);

    std::atomic<bool> push_completed{false};

    std::thread blocker([&queue, &push_completed]() {
        queue.push(3); // здесь поток будет заблокирован
        push_completed = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(push_completed);

    auto val = queue.pop();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 1);

    blocker.join();

    EXPECT_TRUE(push_completed);
    EXPECT_EQ(queue.size(), 2);
}

TEST(ConcurrentQueueTest, BlockingPop) {
    ConcurrentQueue<int> queue;

    std::atomic<bool> pop_completed{false};
    std::optional<int> result;

    std::thread blocker([&queue, &pop_completed, &result]() {
        result = queue.pop(); // Здесь очередь заблокируется, пока в неё не запишут что-нибудь
        pop_completed = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(pop_completed);

    queue.push(123);
    blocker.join();

    EXPECT_TRUE(pop_completed);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 123);
}

TEST(ConcurrentQueueTest, CloseUnblocksWaitingThreads) {
    ConcurrentQueue<int> queue;

    std::atomic<int> pop_count{0};

    // пять потоков блокируются в pop
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; i++) {
        threads.emplace_back([&queue, &pop_count]() {
            auto val = queue.pop();
            if (!val.has_value()) {
                ++pop_count;
            }
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // и очередь закрывается
    queue.close();

    for (auto& t : threads) t.join();

    // все пять потоков должны получить пустое значение и завершиться.
    EXPECT_EQ(pop_count.load(), 5);
}
