cmake_minimum_required(VERSION 4.2)
project(factorization)

set(CMAKE_CXX_STANDARD 20)

add_executable(factorization src/main.cpp
        src/workers/udp_receiver.cpp
        src/workers/prime_numbers_provider.cpp
        src/infrastructure/app_config.cpp
        src/workers/result_performer.cpp
        src/workers/factorizer.cpp
        src/libs/bit_sieve.cpp
)

target_include_directories(factorization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_options(factorization PRIVATE -static-libstdc++ -static-libgcc)