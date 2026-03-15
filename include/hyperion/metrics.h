#pragma once
#include <atomic>
#include <chrono>

namespace hyperion
{

    class Metrics
    {
    public:
        // singleton — one instance shared across all threads
        static Metrics &instance()
        {
            static Metrics m;
            return m;
        }

        // called every time a request is completed
        void record_request()
        {
            total_requests_.fetch_add(1, std::memory_order_relaxed);
        }

        // called every time a connection opens
        void record_connection()
        {
            active_connections_.fetch_add(1, std::memory_order_relaxed);
        }

        // called every time a connection closes
        void record_disconnection()
        {
            active_connections_.fetch_sub(1, std::memory_order_relaxed);
        }

        // getters
        uint64_t total_requests() const
        {
            return total_requests_.load(std::memory_order_relaxed);
        }

        uint64_t active_connections() const
        {
            return active_connections_.load(std::memory_order_relaxed);
        }

        // how long server has been running in seconds
        uint64_t uptime_seconds() const
        {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::seconds>(
                       now - start_time_)
                .count();
        }

        // requests per second calculated from total
        uint64_t requests_per_second() const
        {
            uint64_t uptime = uptime_seconds();
            if (uptime == 0)
                return 0;
            return total_requests() / uptime;
        }

    private:
        Metrics() : start_time_(std::chrono::steady_clock::now()) {}

        std::atomic<uint64_t> total_requests_{0};
        std::atomic<uint64_t> active_connections_{0};
        std::chrono::steady_clock::time_point start_time_;
    };

} // namespace hyperion