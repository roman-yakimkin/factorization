//
// Created by Roman Yakimkin on 26.06.2026.
//

#pragma once

#include <iostream>

#include "defs.h"
#include "libs/concurrent_queue.h"

namespace workers {
    class UDPReceiver {
        std::string m_host;
        uint16_t m_port;
        NumberQueue* m_queue = nullptr;
        int m_sock = -1;
        uint64_t m_min_val;
        uint64_t m_max_val;

        uint64_t ntohll(uint64_t net_val);
        bool receive(uint64_t& number);
    public:
        explicit UDPReceiver(std::string host, uint16_t port, NumberQueue* queue, uint64_t min_val, uint64_t max_val);
        ~UDPReceiver();

        // запретить копирование
        UDPReceiver(const UDPReceiver&) = delete;
        UDPReceiver& operator=(const UDPReceiver&) = delete;

        // разрешить перемещение
        UDPReceiver(UDPReceiver&& another) = default;
        UDPReceiver& operator=(UDPReceiver&& other) noexcept = default;

        void operator()(std::stop_token stoken);
    };
} // workers