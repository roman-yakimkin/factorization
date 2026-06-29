# Build stage
FROM gcc:13 AS builder

# Скачиваем и устанавливаем CMake 4.2.0
ARG CMAKE_VERSION=4.2.0
ARG CMAKE_OS=linux-x86_64
ARG CMAKE_URL=https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-${CMAKE_OS}.tar.gz

RUN apt-get update && apt-get install -y wget ca-certificates \
    && rm -rf /var/lib/apt/lists/* \
    && wget -O cmake.tar.gz "${CMAKE_URL}" \
    && mkdir -p /opt/cmake \
    && tar -xzf cmake.tar.gz -C /opt/cmake --strip-components=1 \
    && rm cmake.tar.gz \
    && ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake \
    && cmake --version

WORKDIR /app

COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release -j$(nproc) \
    && strip build/factorization \
    && ls -l build/factorization

# Runtime stage
FROM debian:bookworm-slim
WORKDIR /app
COPY --from=builder /app/build/factorization ./factorization
EXPOSE 12345/udp
CMD ["/app/factorization"]