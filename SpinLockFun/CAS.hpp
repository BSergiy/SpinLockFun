#pragma once

#include <atomic>
#include <string>

class CAS_SpinLock {
    std::atomic_bool flag;

public:
    void lock() noexcept {

        bool f = false;

        for (;;) {
            if (flag.compare_exchange_weak(f, true, std::memory_order_acquire)) {
                break;
            }

            f = false;

            while (flag.load(std::memory_order_relaxed)) {
                _mm_pause();
            }
        }
    }

    bool try_lock() noexcept {
        bool f = false;

        if (flag.compare_exchange_weak(f, true, std::memory_order_relaxed)) {
            return true;
        }

        return false;
    }

    void unlock() noexcept {
        flag.exchange(false, std::memory_order_release);
    }

    static std::string name() noexcept {
        static std::string n{ "CAS" };

        return n;
    }

    static std::string description() noexcept {
        static std::string n{ "Compare-and-Swap realisation with full optimizations" };

        return n;
    }
};