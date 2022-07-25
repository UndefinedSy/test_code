#ifndef SYAO_LEAKY_BUCKET_H
#define SYAO_LEAKY_BUCKET_H

#include <iostream>

#include <atomic>
#include <chrono>
#include <thread>

#include "assert.h" 

namespace rate_limiter {

typedef std::chrono::steady_clock::time_point steady_time;
typedef std::chrono::microseconds ms;

struct State {
	steady_time last_;
	std::chrono::microseconds sleep_for_;

    State() = default;
    State(steady_time last, std::chrono::microseconds sleep_for)
        : last_(last), sleep_for_(sleep_for) {}
};

class SteadyBucketRateLimiter {
public:
    explicit SteadyBucketRateLimiter(int64_t rate, int32_t slack)
        : state_p_(new State(std::chrono::steady_clock::now(), std::chrono::microseconds(0)))
        , per_request_(std::chrono::microseconds(1000000 / rate))
        , max_slack_(-1 * per_request_ * slack) {
            std::cout << "per_request_: " << per_request_.count() << "(us)\n";
        }
    ~SteadyBucketRateLimiter()
    {
        if (state_p_ != nullptr) {
            delete state_p_;
        }
        state_p_ = nullptr;
    }

private:
	std::atomic<State*> state_p_;
    // padding?

	std::chrono::microseconds per_request_;
    std::chrono::microseconds max_slack_;

public:
    // Consume n tokens
    bool Acquire(int64_t n)
    {
        assert(n > 0);

        auto new_state = new State();
        bool taken = false;
        // std::chrono::microseconds interval = std::chrono::microseconds(0);
        std::chrono::milliseconds interval = std::chrono::milliseconds(0);
        State* old_state = nullptr;

        while (!taken)
        {
            auto now = std::chrono::steady_clock::now();
            old_state = state_p_.load(std::memory_order_relaxed);
            new_state->last_ = now;
            new_state->sleep_for_ = old_state->sleep_for_;

            new_state->sleep_for_ += (n * per_request_);
            new_state->sleep_for_ -= std::chrono::duration_cast<
                std::chrono::microseconds>(now - old_state->last_);
            
            if (new_state->sleep_for_ < max_slack_)
            {
                new_state->sleep_for_ = max_slack_;
            }

            if (new_state->sleep_for_ > std::chrono::microseconds(0))
            {
                new_state->last_ = new_state->last_ + new_state->sleep_for_;
                interval = std::chrono::duration_cast<std::chrono::milliseconds>(new_state->sleep_for_);
                new_state->sleep_for_ = std::chrono::microseconds(0);
            }
            taken = state_p_.compare_exchange_strong(old_state, new_state,
                std::memory_order_release, std::memory_order_relaxed);
        }

        if (taken)
        {
            if (interval > std::chrono::milliseconds(0))
            {
                std::this_thread::sleep_for(interval);
            }
            delete old_state;
        }
        return true;
    }

    bool Acquire() { return Acquire(1); }
};

} // namespace rate_limiter

#endif