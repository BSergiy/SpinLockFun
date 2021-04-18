#pragma once

#include <mutex>
#include <string>

class MutexLock {
    std::mutex m;

public:
    void lock() noexcept {
        m.lock();
    }

    void unlock() noexcept {
        m.unlock();
    }

    static std::string name() noexcept {
        static std::string n{ "Mutex" };

        return n;
    }

    static std::string description() noexcept {
        static std::string n{ "Spinlock on mutex" };

        return n;
    }
};