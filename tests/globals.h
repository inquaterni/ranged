//
// Created by mmatz on 9/5/25.
//

#ifndef GLOBALS_H
#define GLOBALS_H
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

using TestNameT = std::pair<std::string, std::string>;

static std::vector<std::pair<TestNameT, void (*)()>> test_v;

inline std::ostream& operator<<(std::ostream& os, const std::chrono::seconds& t) {
    return os << t.count() << "s";
}
inline std::ostream &operator<<(std::ostream &os, const std::chrono::milliseconds &ms) {
    return os << static_cast<int>(ms.count()) << "ms";
}
inline std::ostream &operator<<(std::ostream &os, const std::chrono::microseconds &ms) {
    return os << static_cast<int>(ms.count()) << "us";
}
inline std::ostream &operator<<(std::ostream &os, const std::chrono::nanoseconds &ns) {
    return os << static_cast<int>(ns.count()) << "ns";
}

struct dispatcher {
    template <typename FuncT>
    dispatcher(const TestNameT& name, const FuncT &func) {
        test_v.emplace_back(name, func);
    }

    template <typename DurationT>
    static void run_tests() {
        std::sort(test_v.begin(), test_v.end());
#if __cplusplus >= 201703L
        std::string old_name;
        for (const auto &[name, test]: test_v) {
            if (old_name != name.first) {
                std::cout << "Test case: " << name.first << "\n";
                old_name = name.first;
            }
            std::cout << "\tRunning test " << name.second << "...";
            const auto start = std::chrono::high_resolution_clock::now();
            test();
            const auto elapsed = std::chrono::high_resolution_clock::now() - start;
            std::cout << ' ' << std::chrono::duration_cast<DurationT>(elapsed) << " \033[32mOK\033[0m\n";
        }
#else
        std::string last_name;
        for (const auto &pair: test_v) {
            if (last_name != pair.first.first) {
                std::cout << "Test case: " << pair.first.first << "\n";
                last_name = pair.first.first;
            }
            std::cout << "\tRunning test " << pair.first.second << "...";
            const auto start = std::chrono::high_resolution_clock::now();
            pair.second();
            const auto elapsed = std::chrono::high_resolution_clock::now() - start;
            std::cout << ' ' << std::chrono::duration_cast<DurationT>(elapsed) << " \033[32mOK\033[0m\n";
        }
#endif
        std::cout << "\033[32mPASSED\033[0m" << std::endl;
    }
};

#define TEST(test_suite, test_name) \
    void test_suite## _## test_name(); \
    int dummy_## test_suite## _## test_name = (dispatcher(std::make_pair(#test_suite, #test_name), &test_suite## _## test_name), 0); \
    void test_suite## _## test_name()


#endif //GLOBALS_H
