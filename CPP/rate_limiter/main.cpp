#include "leaky_bucket.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    rate_limiter::SteadyBucketRateLimiter limiter(2000, 10);
    
    limiter.Acquire();
    // prepare 4.5 request
    std::this_thread::sleep_for(std::chrono::milliseconds(450));

    std::vector<std::thread> threads;
    threads.reserve(20);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 100; ++i) {
        threads.emplace_back([&limiter, i]() {
            auto prev = std::chrono::steady_clock::now();
            limiter.Acquire(30);
            auto now = std::chrono::steady_clock::now();
            std::cout << i << " Acquired, cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(now - prev).count() << "(ms)\n";
            prev = now;
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    auto end = std::chrono::steady_clock::now();
    // expect spend 1.5s
    std::cout << "Total cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "(ms)\n";
    return 0;
}