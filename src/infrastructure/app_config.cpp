//
// Created by Roman Yakimkin on 26.06.2026.
//

#include "app_config.h"

namespace infra {
    AppConfig::AppConfig() {
        const char* env_host = std::getenv("HOST");
        const char* env_port = std::getenv("PORT");
        const char* env_min_val = std::getenv("MIN_VAL");
        const char* env_max_val = std::getenv("MAX_VAL");

        m_host = env_host ? env_host : "0.0.0.0";
        m_port = env_port ? std::stoi(env_port) : 12345;
        m_min_val = env_min_val ? std::stoull(env_min_val) : 1;
        m_max_val = env_max_val ? std::stoull(env_max_val) : 1ull << 50;
    }

    AppConfig::AppConfig(std::string host, const uint16_t port, const uint64_t min_val, const uint64_t max_val)
    : m_host(std::move(host)), m_port(port), m_min_val(min_val), m_max_val(max_val) {}

    std::string AppConfig::host() {
        return m_host;
    }

    uint16_t AppConfig::port() const {
        return m_port;
    }

    uint64_t AppConfig::min_val() const {
        return m_min_val;
    }

    uint64_t AppConfig::max_val() const {
        return m_max_val;
    }
} // infra