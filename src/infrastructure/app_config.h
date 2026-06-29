//
// Created by Roman Yakimkin on 26.06.2026.
//

#pragma once

#include <string>
#include <cstdint>
#include <utility>

namespace infra {
    class AppConfig {
        std::string m_host = "localhost";
        uint16_t m_port = 12345;
        uint64_t m_min_val = 0;
        uint64_t m_max_val = 1ull << 40;
    public:
        AppConfig();
        AppConfig(std::string host, uint16_t port, uint64_t min_val, uint64_t max_val);

        std::string host();
        uint16_t port() const;
        uint64_t min_val() const;
        uint64_t max_val() const;
    };
} // infra