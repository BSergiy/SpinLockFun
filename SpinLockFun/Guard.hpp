#pragma once

template<class TSPIN>
class Guard {
    TSPIN& _obj;

public:
    Guard(TSPIN& flag) noexcept : _obj(flag) {
        _obj.lock();
    }

    ~Guard() noexcept {
        _obj.unlock();
    }
};

////////////////////////////////////////////////////////////////////////////////

template<class TSPIN>
class TryGuard {
    TSPIN& _obj;

public:
    TryGuard(TSPIN& flag) noexcept : _obj(flag) {
        if (!_obj.try_lock()) {
            _obj.lock();
        }
    }

    ~TryGuard() noexcept {
        _obj.unlock();
    }
};