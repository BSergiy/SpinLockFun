#pragma once

#include <atomic>
#include <string>

class NTAS_SpinLock {
    std::atomic_bool flag {false};

public:
    void lock() noexcept {
        while (flag.exchange(true));
    }

    void unlock() noexcept {
        flag.exchange(false);
    }

    static std::string name() noexcept {
        static std::string n{ "Bad TAS" };

        return n;
    }

    static std::string description() noexcept {
        static std::string n{ "Poor Test-and-Set realisation" };

        return n;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class TAS_SpinLock {
    std::atomic_bool flag{ false };

public:
    void lock() noexcept {
        while (flag.exchange(true, std::memory_order_acquire)) {
            _mm_pause();
        }
    }

    void unlock() noexcept {
        flag.exchange(false, std::memory_order_release);
    }

    static std::string name() noexcept {
        static std::string n{ "TAS" };

        return n;
    }

    static std::string description() noexcept {
        static std::string n{ "Test-and-Set realisation with memory ordering and CPU delay" };

        return n;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class OTAS_spinlock {
    std::atomic_bool flag = { false };

public:
    void lock() noexcept {
        for (;;) {
            // Optimistically assume the lock is free on the first try
            if (!flag.exchange(true, std::memory_order_acquire)) {
                return;
            }
            // Wait for lock to be released without generating cache misses
            while (flag.load(std::memory_order_relaxed)) {
                // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
                // hyper-threads
                _mm_pause();
            }
        }
    }

    bool try_lock() noexcept {
        // First do a relaxed load to check if lock is free in order to prevent
        // unnecessary cache misses if someone does while(!try_lock())
        return !flag.load(std::memory_order_relaxed) &&
            !flag.exchange(true, std::memory_order_acquire);
    }

    void unlock() noexcept {
        flag.store(false, std::memory_order_release);
    }

    static std::string name() noexcept {
        static std::string n{ "Opt TAS" };

        return n;
    }

    static std::string description() noexcept {
        static std::string n{ "Test-and-Set realisation with full optimizations" };

        return n;
    }
};