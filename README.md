# Hyperion HTTP Server

High-performance HTTP server built from scratch in C++17 — **215,000+ req/sec**, sub-3ms p99 latency.

## Stack

`C++17` `kqueue` `CMake` `POSIX Sockets` `HTTP/1.1`

## Features

- kqueue event loop with io_uring-inspired proactor architecture (io_uring is Linux-only, kqueue is the macOS equivalent)
- Non-blocking I/O state machine + SO_REUSEPORT multi-core
- Zero-allocation connection pool + in-memory file cache
- Lock-free metrics with live dashboard

## Build & Run

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) && cd ..

ulimit -n 65535
./build/hyperion
```

## Benchmark

```bash
wrk -t4 -c500 -d30s --latency http://localhost:8080/bench
```
