//
// Created by Roman Yakimkin on 25.06.2026.
//

#pragma once
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "core/contracts/i_queue.h"

template <typename T>
class ConcurrentQueue : public core::IQueue<T> {
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv_not_empty;
    std::condition_variable m_cv_not_full;
    size_t m_max_size = 0;
    bool m_is_closed = false;
public:
    explicit ConcurrentQueue(size_t max_size = 0) : m_max_size(max_size) {}
    ConcurrentQueue(const ConcurrentQueue&) = delete;
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

    bool push(T value) override {
        std::unique_lock lock(m_mutex);

        if (m_max_size > 0) {
            m_cv_not_full.wait(lock, [this] {return m_queue.size() < m_max_size || m_is_closed; });
        }

        if (m_is_closed) return false;

        m_queue.push(std::move(value));

        m_cv_not_empty.notify_one();

        return true;
    }

    std::optional<T> pop() override {
        std::unique_lock lock(m_mutex);

        m_cv_not_empty.wait(lock, [this] {return !m_queue.empty() || m_is_closed; });

        // Если очередь пуста и закрыта, то возвращаем ничего
        if (m_queue.empty() && m_is_closed) return std::nullopt;

        T value = std::move(m_queue.front());
        m_queue.pop();

        if (m_max_size > 0) {
            m_cv_not_full.notify_one();
        }

        return value;
    }

    bool empty() const override {
        std::lock_guard lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const override {
        std::lock_guard lock(m_mutex);
        return m_queue.size();
    }

    void close() override {
        std::lock_guard lock(m_mutex);
        m_is_closed = true;

        // Разбудить все потоки. которые ждут в pop()
        m_cv_not_empty.notify_all();

        // Разбудить все потоки. которые ждут в push()
        m_cv_not_full.notify_all();
    }
};
