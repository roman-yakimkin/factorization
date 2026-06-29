# Factorization Server

Многопоточный сервер для разложения больших чисел на простые множители. Принимает числа по UDP-протоколу, распараллеливает вычисления на несколько потоков и выводит результаты в реальном времени.

## 🎯 Назначение проекта

Демонстрационный проект, показывающий навыки работы с:
- Многопоточностью в C++20 (`std::jthread`, `std::stop_token`)
- Сетевым программированием (UDP сокеты)
- Потокобезопасными очередями
- Оптимизированными алгоритмами (битовое решето)
- Docker-контейнеризацией

## 🏗️ Архитектура

Приложение построено по конвейерной архитектуре с разделением ответственности:

```
UDP Receiver → Number Queue → Factorizer Pool → Result Queue → Result Performer
                              ↑
              Prime Numbers Provider (битовое решето)
```

### Компоненты:

- **UDPReceiver** — принимает 64-битные числа по UDP, валидирует диапазон, помещает в очередь
- **PrimeNumbersProvider** — генерирует простые числа до √(max_value) с использованием оптимизированного битового решета
- **Factorizer** (пул потоков) — раскладывает числа на простые множители, используя предвычисленные простые числа
- **ResultPerformer** — выводит результаты разложения с указанием затраченного времени (в микросекундах)
- **ConcurrentQueue** — потокобезопасная очередь с возможностью graceful shutdown

## 🚀 Быстрый старт

### Требования
- C++20 совместимый компилятор (GCC 10+, Clang 12+)
- CMake 4.2+
- Docker и Docker Compose (опционально)

### Сборка и запуск

```bash
# Клонирование репозитория
git clone https://github.com/roman-yakimkin/factorization.git
cd factorization

# Сборка через CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Запуск сервера
./build/factorization
```

### Запуск через Docker Compose

```bash
# Запуск только сервера
docker-compose up -d

# Запуск сервера + тестового отправителя
docker-compose --profile sender up -d
```

## ⚙️ Конфигурация

Сервер настраивается через переменные окружения:

| Переменная | Описание | По умолчанию                  |
|------------|----------|-------------------------------|
| `HOST` | Хост для прослушивания UDP | `0.0.0.0`                     |
| `PORT` | Порт для UDP | `12345`                       |
| `MIN_VAL` | Минимальное допустимое значение | `2`                           |
| `MAX_VAL` | Максимальное допустимое значение | `2⁵⁰ (1 125 899 906 842 624)` |
| `THREADS` | Количество потоков факторизации | Авто (по числу ядер)          |

## 📡 Протокол обмена

Сервер принимает UDP-пакеты размером ровно **8 байт** (64-битное беззнаковое целое в network byte order).

### Тестовый отправитель

В папке `udp-sender/` находится Python-скрипт для тестирования:

```bash
cd udp-sender
python3 sender.py --mode mixed --count 100
```

Режимы работы:
- `random` — случайные числа в заданном диапазоне
- `sequential` — последовательные числа
- `mixed` — комбинация различных паттернов

## 🔧 Технические детали

### Многопоточность (C++20)
- `std::jthread` с автоматическим `join` в деструкторе — потоки гарантированно завершаются при уничтожении объекта
- `std::stop_token` для graceful shutdown потоков
- Потокобезопасные очереди на основе `std::mutex` и `std::condition_variable`

### Оптимизация алгоритма
- **Битовое решето** (`BitSieve`) — компактное хранение состояния (1 бит на число)
- Предвычисление простых чисел до √(max_value)
- Пул потоков факторизации для параллельной обработки

### Сетевое взаимодействие
- UDP сокеты с таймаутом на чтение для возможности graceful shutdown
- Конвертация network byte order → host byte order
- Валидация размера пакета и диапазона значений

### Graceful Shutdown
- Обработка сигналов `SIGINT` и `SIGTERM`
- Каскадное завершение потоков через закрытие очередей
- Автоматическое освобождение ресурсов через RAII

## 📂 Структура проекта

```
factorization/
├── src/
│   ├── core/              # Базовые модели данных
│   ├── helpers/           # Утилиты (определение числа ядер)
│   ├── infrastructure/    # Конфигурация приложения
│   ├── libs/              # Битовое решето
│   ├── workers/           # Рабочие потоки
│   │   ├── udp_receiver.cpp
│   │   ├── factorizer.cpp
│   │   ├── result_performer.cpp
│   │   └── prime_numbers_provider.cpp
│   └── main.cpp
├── udp-sender/            # Тестовый Python-отправитель
├── CMakeLists.txt
├── docker-compose.yml
└── makefile
```

## 📊 Пример вывода

<details>
<summary>📺 Показать пример вывода сервера</summary>

```
factorization-server  | Starting factorization server...
factorization-server  | UDP post listening 12345...
factorization-server  | UDP Listener thread started.
factorization-server  | Prime numbers provider thread started.
factorization-server  | Factorizer thread #1, native id: 140498563053248 started.
factorization-server  | Factorizer thread #2, native id: 140498446579392 started.
factorization-server  | Factorizer thread #3, native id: 140498550462144 started.
factorization-server  | Factorizer thread #4, native id: 140498542069440 started.
factorization-server  | Factorizer thread #5, native id: 140498533676736 started.
factorization-server  | Factorizer thread #6, native id: 140498525284032 started.
factorization-server  | Factorizer thread #7, native id: 140498438186688 started.
factorization-server  | Factorizer thread #8, native id: 140498429793984 started.
factorization-server  | Factorizer thread #9, native id: 140498421401280 started.
factorization-server  | Factorizer thread #10, native id: 140498413008576 started.
factorization-server  | Factorizer thread #11, native id: 140498404615872 started.
factorization-server  | Factorizer thread #12, native id: 140498396223168 started.
factorization-server  | Factorizer thread #13, native id: 140498387830464 started.
factorization-server  | Factorizer thread #14, native id: 140498379437760 started.
factorization-server  | Factorizer thread #15, native id: 140498371045056 started.
factorization-server  | Result performer thread started.
factorization-server  | Thread #2
factorization-server  | Number : 42
factorization-server  | Factors : 2, 3, 7
factorization-server  | Duration : 3mks. 
factorization-server  | Thread #6
factorization-server  | Number : 97
factorization-server  | Factors : 97
factorization-server  | Duration : 100mks. 
factorization-server  | Thread #11
factorization-server  | Number : 1024
factorization-server  | Factors : 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
factorization-server  | Duration : 62mks. 
factorization-server  | Thread #13
factorization-server  | Number : 3628800
factorization-server  | Factors : 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 5, 5, 7
factorization-server  | Duration : 156mks. 
factorization-server  | Thread #12
factorization-server  | Number : 999999999989
factorization-server  | Factors : 999999999989
factorization-server  | Duration : 203mks. 
factorization-server  | Thread #4
factorization-server  | Number : 2147483647
factorization-server  | Factors : 2147483647
factorization-server  | Duration : 147mks. 
factorization-server  | Thread #15
factorization-server  | Number : 123456789012345
factorization-server  | Factors : 3, 5, 283, 3851, 7552031
factorization-server  | Duration : 88mks. 
factorization-server  | Thread #10
factorization-server  | Number : 1099511627776
factorization-server  | Factors : 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
factorization-server  | Duration : 141mks. 
Gracefully Stopping... press Ctrl+C again to force
Container factorization-server Stopping 
factorization-server  | 
factorization-server  | Graceful shutdown initiated...
factorization-server  | All threads stopped successfully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | Result performer thread stopped gracefully.
factorization-server  | UDP Listener thread stopped gracefully.
```
</details>

## 🛠️ Технологии

- **Язык:** C++20
- **Сборка:** CMake 4.2+
- **Контейнеризация:** Docker, Docker Compose
- **Тестирование:** Python 3 (udp-sender)
- **ОС:** Linux (POSIX sockets)

## 📝 Лицензия

Этот проект создан в демонстрационных целях.