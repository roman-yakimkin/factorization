//
// Created by Roman Yakimkin on 26.06.2026.
//

#pragma once
#include <cstddef>
#include <optional>

namespace core {
    template <typename T>
    class IQueue {
    public:
        virtual ~IQueue() = default;

        virtual bool push(T value) = 0;
        virtual std::optional<T> pop() = 0;
        [[nodiscard]] virtual bool empty() const = 0;
        [[nodiscard]] virtual std::size_t size() const = 0;
        virtual void close() = 0;
    };
}
