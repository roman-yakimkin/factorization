//
// Created by Roman Yakimkin on 26.06.2026.
//

#include "udp_receiver.h"

#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

namespace workers {
    UDPReceiver::UDPReceiver(std::string host, uint16_t port, NumberQueue* queue, uint64_t min_val, uint64_t max_val)
            : m_host(std::move(host)), m_port(port), m_queue(queue), m_min_val(min_val), m_max_val(max_val) {
        m_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_sock < 0) {
            throw std::runtime_error("Socket creating error");
        }

        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;

        if (setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            close(m_sock);
            throw std::runtime_error("Failed to set socket timeout");
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(m_port);

        if (m_host.empty() || m_host == "0.0.0.0") {
            addr.sin_addr.s_addr = INADDR_ANY;
        } else {
            if (inet_pton(AF_INET, m_host.c_str(), &addr.sin_addr) <= 0) {
                close(m_sock);
                throw std::runtime_error("Incorrect addr: " + host);
            }
        }

        if (bind(m_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            close(m_sock);
            throw std::runtime_error("Bind error");
        }

        std::cout << "UDP post listening " << m_port << "...\n";
    }

    UDPReceiver::~UDPReceiver() {
        if (m_sock >= 0) {
            close(m_sock);
            m_sock = -1;
        }
    }

    bool UDPReceiver::receive(uint64_t &number) {
        uint8_t buf[8];
        sockaddr_in sender_addr{};
        socklen_t sender_len = sizeof(sender_addr);

        ssize_t bytes = recvfrom(m_sock, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&sender_addr), &sender_len);

        if (bytes < 0) {

            // таймаут
            if (errno == EAGAIN || errno == EWOULDBLOCK) return false;

            // реальная ошибка сокета
            throw std::runtime_error("recvfrom error: " + std::string(strerror(errno)));
        }

        // неправильный размер пакета
        if (bytes != 8) {
            return false;
        }

        // сборка чисел из байт
        uint64_t net_value;
        std::memcpy(&net_value, buf, 8);
        number = ntohll(net_value);

        return true;
    }

    void UDPReceiver::operator()(std::stop_token stoken) {
        std::cout << "UDP Listener thread started.\n";

        try {
            while (!stoken.stop_requested()) {
                uint64_t number;
                if (receive(number)) {
                    if (number < m_min_val || number >= m_max_val) continue;

                    if (!m_queue->push(number)) {
                        std::cout << "Queue closed, stopping UDP receiver.\n";
                        break;
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "UDP Listener Fatal error " << e.what() << "\n";
        }
        catch (...) {
            std::cerr << "[Thread] Unknown fatal error.\n";
        }

        std::cout << "UDP Listener thread stopped gracefully.\n";
    }

    uint64_t UDPReceiver::ntohll(uint64_t net_val) {
        uint64_t host_val;
        uint8_t* bytes = reinterpret_cast<uint8_t *>(&host_val);

        bytes[0] = (net_val >> 56) & 0xFF;
        bytes[1] = (net_val >> 48) & 0xFF;
        bytes[2] = (net_val >> 40) & 0xFF;
        bytes[3] = (net_val >> 32) & 0xFF;
        bytes[4] = (net_val >> 24) & 0xFF;
        bytes[5] = (net_val >> 16) & 0xFF;
        bytes[6] = (net_val >>  8) & 0xFF;
        bytes[7] = (net_val >>  0) & 0xFF;

        return host_val;
    }
} // workers