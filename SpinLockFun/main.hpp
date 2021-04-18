#include <iostream>
#include <vector>
#include <array>
#include <string_view>
#include <thread>
#include <functional>
#include <chrono>
#include <string>
#include <mutex>
#include <algorithm>
#include <Windows.h>

#include "TAS.hpp"
#include "CAS.hpp"
#include "Poor.hpp"
#include "Guard.hpp"

using namespace std;

struct BenchInfo {
    string Name{};
    string Description{};
    int micSec = 0;

    string toString() const noexcept {
        return Name + "\t" + to_string(micSec) /* + "mics" <<*/;
    }

    void println() {
        cout << toString() << endl;
    }

    void println(int etalon) {
        auto procent = static_cast<int>((1 - static_cast<double>(etalon) / micSec) * 100);

        cout << toString() << "(slow on ~" << procent << "%) '" << Description << "'" << endl;
    }
};

constexpr int matrix_size = 100;

std::array<std::array<int, matrix_size>, matrix_size> m1;
std::array<std::array<int, matrix_size>, matrix_size> m2;

void init() {
    for (size_t i = 0; i < matrix_size; ++i) {
        for (size_t j = 0; j < matrix_size; ++j) {
            m1[i][j] = static_cast<int>(i * j);
            m2[i][j] = static_cast<int>(i + j);
        }
    }
}

void calc() {
    for (size_t i = 0; i < matrix_size; ++i) {
        for (size_t j = 0; j < matrix_size; ++j) {
            m1[i][j] += m2[j][i];
        }
    }
}

BenchInfo bench(string_view name, string_view description, size_t count, bool show, function<void()> f) {
    using namespace std::chrono;

    const size_t size = count;
    double avg = .0;


    try {
        while (count--) {
            init();

            const auto start = chrono::high_resolution_clock::now();

            f();

            const auto stop = chrono::high_resolution_clock::now();

            const auto ms = duration_cast<microseconds>(stop - start).count();

            avg += ms;
        }
    }
    catch (exception e) {
        cout << "Fatal error in case '" << name << "': " << e.what() << endl;
        return BenchInfo{};
    }

    BenchInfo info
    {
        .Name = string(name),
        .Description = string(description),
        .micSec = static_cast<int>(::round(avg / size))
    };

    if (show) {
        info.println();
    }

    return info;
}

template <class TSPIN, typename GUARD = Guard<TSPIN>>
function<void()> make_case(size_t iterCount, size_t threadCount) {
    return [=]
    {
        TSPIN spin;

        vector<thread> v;
        v.reserve(threadCount);

        for (size_t i = 0; i < threadCount; ++i) {
            v.emplace_back(
                [&spin, iterCount]
            {
                for (size_t i = 0; i < iterCount; ++i) {
                    GUARD guard(spin);
                    
                    calc();
                }
            });
        }

        for (auto& t : v) {
            t.join();
        }
    };
}

int main()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 8);

    const size_t ALL_ITER_COUNT = 100000;

    const size_t thread_count = 12;
    const size_t iteration_count_per_thread = ALL_ITER_COUNT / thread_count;
    const size_t bench_count = 5;
    const bool showProgress = true;

#define RUN_BENCH( SPINLOCK ) \
    bench(SPINLOCK::name(), SPINLOCK::description(), bench_count, showProgress, make_case<SPINLOCK>(iteration_count_per_thread, thread_count))

#define RUN_BENCH_WITH_TRY_GUARD( SPINLOCK ) \
    bench(SPINLOCK::name(), SPINLOCK::description(), bench_count, showProgress, make_case<SPINLOCK, TryGuard<SPINLOCK>>(iteration_count_per_thread, thread_count))

    vector<BenchInfo> results {
        RUN_BENCH(TAS_SpinLock),
        RUN_BENCH(OTAS_spinlock),
        RUN_BENCH_WITH_TRY_GUARD(OTAS_spinlock),
        RUN_BENCH(NTAS_SpinLock),
        RUN_BENCH(CAS_SpinLock),
        RUN_BENCH_WITH_TRY_GUARD(CAS_SpinLock),
        RUN_BENCH(MutexLock),
    };

#undef MAKE_BENCH
#undef MAKE_BENCH_WITH_TRY_GUARD
    
    sort(results.begin(), results.end(), [](const auto& i1, const auto& i2) {
        return i1.micSec < i2.micSec;
    });

    cout << "------------------SORTED------------------" << endl;

    const auto min = results.begin()->micSec;

    bool flag = false;

    for (auto& i : results) {
        if (flag) {
            SetConsoleTextAttribute(hConsole, 3);
        }
        else {
            SetConsoleTextAttribute(hConsole, 2);
        }

        flag = !flag;
        
        i.println(min);
    }

    SetConsoleTextAttribute(hConsole, 7);
}
