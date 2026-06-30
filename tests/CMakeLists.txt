add_executable(factorization_test
        libs/bit_array_test.cpp
        workers/bit_sieve_test.cpp
        ${CMAKE_SOURCE_DIR}/src/libs/bit_array.cpp
        ${CMAKE_SOURCE_DIR}/src/workers/bit_sieve.h
        libs/test_concurrent_queue.cpp
        ${CMAKE_SOURCE_DIR}/src/workers/prime_numbers_provider.cpp
        workers/prime_numbers_provider_test.cpp
)

target_include_directories(factorization_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(factorization_test
        PRIVATE
        GTest::gtest_main
        GTest::gmock
)

find_package(Threads REQUIRED)
target_link_libraries(factorization_test PRIVATE Threads::Threads)

# Интеграция с CTest, чтобы тесты можно было запускать общей командой ctest
include(GoogleTest)
gtest_discover_tests(factorization_test)