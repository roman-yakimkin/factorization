#!/usr/bin/env python3
import os
import socket
import struct
import time
import random
import argparse

def send_number(sock, number, addr):
    """Отправляет число в сетевом порядке (Big-Endian)"""
    data = struct.pack('!Q', number)  # !Q = unsigned long long, network order
    sock.sendto(data, addr)
    print(f"Отправлено: {number:,} (0x{number:X})")

def demo_simple(sock, addr):
    """Простая демонстрация: числа от 1 до 1000"""
    print("\n=== Простые числа (1-1000) ===")
    for i in range(1, 1001):
        send_number(sock, i, addr)
        time.sleep(0.02)

def demo_mixed(sock, addr):
    """Смешанная демонстрация: разные типы чисел"""
    test_cases = [
        # (число, описание)
        (42, "Маленькое составное"),
        (97, "Простое число"),
        (1024, "Степень двойки"),
        (3628800, "10! (факториал)"),
        (999999999989, "Большое простое"),
        (2**31 - 1, "Mersenne prime (2^31-1)"),
        (123456789012345, "Большое составное"),
        (2**40, "Степень двойки (2^40)"),
    ]

    print("\n=== Разнообразные числа ===")
    for number, desc in test_cases:
        print(f"\n[{desc}]")
        send_number(sock, number, addr)
        time.sleep(1.0)

def demo_burst(sock, addr, count=50):
    """Пакетная отправка (демонстрация очереди и потоков)"""
    print(f"\n=== Burst: {count} случайных чисел ===")
    for i in range(count):
        number = random.randint(1, 10**15)
        send_number(sock, number, addr)
        # Небольшая задержка, чтобы не перегрузить
        time.sleep(0.1)

    print("Burst завершен!")

def demo_stress(sock, addr, duration_sec=10):
    """Стресс-тест: максимальная скорость отправки"""
    print(f"\n=== Стресс-тест на {duration_sec} секунд ===")
    start = time.time()
    count = 0

    while time.time() - start < duration_sec:
        number = random.randint(1, 10**9)
        send_number(sock, number, addr)
        count += 1

    elapsed = time.time() - start
    print(f"Отправлено {count} чисел за {elapsed:.2f} сек ({count/elapsed:.1f} чисел/сек)")

def main():
    parser = argparse.ArgumentParser(description='UDP Sender для демонстрации факторизации')
    parser.add_argument('--host', default=os.getenv('SERVER_HOST', '127.0.0.1'))
    parser.add_argument('--port', type=int, default=int(os.getenv('SERVER_PORT', 12345)))
    parser.add_argument('--mode', choices=['simple', 'mixed', 'burst', 'stress'],
                        default='mixed', help='Режим демонстрации')
    parser.add_argument('--count', type=int, default=50, help='Количество чисел для burst')
    parser.add_argument('--duration', type=int, default=10, help='Длительность стресс-теста (сек)')

    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    addr = (args.host, args.port)

    try:
        if args.mode == 'simple':
            demo_simple(sock, addr)
        elif args.mode == 'mixed':
            demo_mixed(sock, addr)
        elif args.mode == 'burst':
            demo_burst(sock, addr, args.count)
        elif args.mode == 'stress':
            demo_stress(sock, addr, args.duration)
    finally:
        sock.close()
        print("\nГотово!")

if __name__ == '__main__':
    main()