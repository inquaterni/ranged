//
// Created by mmatz on 9/4/25.
//
#include <cassert>
#include <deque>
#include <array>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <string>

#include "globals.h"
#define RANGED_IMPLEMENTATION
#define RANGED_NO_DEPRECATION_WARNINGS 1
#include <functional>


#include "ranged.h"

template<typename T>
bool operator==(const std::vector<T> &lhs, const std::vector<T> &rhs) {
    if (lhs.size() != rhs.size()) return false;

    for (size_t i = 0; i < lhs.size(); i++) {
        if (lhs[i] != rhs[i]) return false;
    }

    return true;
}

TEST(vector, any_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const bool result = ranged::any(v, [](const int &i) { return i == 7; });
    assert(result);
}

TEST(vector, all_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const bool result = ranged::all(v, [](const int &i) { return i == 7; });
    assert(result == false);
}

TEST(vector, contains_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const bool result = ranged::contains(v, 7);
    assert(result == true);
}

TEST(vector, count_if_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const auto result = ranged::count_if(v, [](const int &i) { return i > 5; });
    assert(result == 5);
}

TEST(vector, first_or_default_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const auto result = ranged::first_or_default(v, [](const int &i) { return i > 5; });
    assert(result == 6);
}

TEST(vector, for_each_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5};
    int sum = 0;
    ranged::for_each(v, [&sum](const int &x) { sum += x; });
    assert(sum == 15);
}

TEST(vector, to_vector_test) {
    const std::vector<int> v = {1, 2, 3};
    const auto vv = ranged::to_vector<std::vector<int>, std::allocator<int>>(v);
    assert(vv == v);
}

TEST(vector, to_array_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5};
    const auto a = ranged::to_array<5>(v);
    assert(a[0] == 1 && a[4] == 5);
}

TEST(vector, max_test) {
    const std::vector<int> v = {1, 3, 2, 7, 4};
    const int m = ranged::max(v);
    assert(m == 7);
}

TEST(vector, min_test_descending_input) {
    const std::vector<int> v = {9, 7, 5, 3, 1};
    const int m = ranged::min(v);
    assert(m == 1);
}

TEST(vector, filter_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const std::vector<int> expected = {6, 7, 8, 9, 10};
    const auto result = ranged::filter(v, [](const int &i) { return i > 5; });
    assert(result == expected);
}

TEST(vector, select_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const std::vector<std::string> expected = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    const auto result = ranged::select(v, [](const int &i) {return std::to_string(i);});
    assert(result == expected);
}

TEST(vector, select_squares_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5};
    const std::vector<int> expected = {1, 4, 9, 16, 25};
    const auto result = ranged::select(v, [](const int &i) {return i * i;});
    assert(result == expected);
}

TEST(vector, select_doubles_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5};
    const std::vector<double> expected = {1.0, 2.0, 3.0, 4.0, 5.0};
    const auto result = ranged::select(v, [](const int &i) {return static_cast<double>(i);});
    assert(result == expected);
}

// deque tests (random access similar to vector)
TEST(deque, basic_ops) {
    const std::deque<int> d = {1, 2, 3, 4, 5, 6};
    assert(ranged::any(d, [](const int &x) { return x == 4; }));
    assert(ranged::all(d, [](const int &x) { return x >= 1; }));
    assert(ranged::contains(d, 6));
    assert(ranged::count_if(d, [](const int &x) { return x % 2 == 0; }) == 3);
    const int first_over_3 = ranged::first_or_default(d, [](const int &x) { return x > 3; });
    assert(first_over_3 == 4);
}

TEST(deque, to_array_and_max_min) {
    const std::deque<int> d = {5, 4, 9, 1, 3};
    const auto a = ranged::to_array<5>(d);
    assert(a[2] == 9);
    assert(ranged::max(d) == 9);
    const std::deque<int> d2 = {9, 7, 5, 3, 2};
    assert(ranged::min(d2) == 2);
}

TEST(deque, filter_test) {
    const std::deque<int> d = {1, 2, 3, 4, 5, 6};
    const auto res = ranged::filter(d, std::function<bool(const int &)>([](const int &x) { return x > 3; }));
    assert(res.size() == 3);
    assert(res.at(0) == 4 && res.at(2) == 6);
}

TEST(deque, select_test) {
    const std::deque<int> d = {1, 2, 3, 4, 5};
    const std::deque<std::string> expected = {"1", "2", "3", "4", "5"};
    const auto result = ranged::select(d, std::function<std::string(const int &)>([](const int &i) {return std::to_string(i);}));
    assert(result == expected);
}

// array tests (only functions explicitly overloaded or iterator-based where constraints allow)
TEST(array, filter_test) {
    const std::array<int, 6> a = {{1, 2, 3, 4, 5, 6}};
    const std::vector<int> expected = {4, 5, 6};
    const auto res = ranged::filter<int, 6>(a, [](const int &x) { return x > 3; });
    assert(res == expected);
}

// set tests (works for iterator-based ops; avoid max/min/to_array which need at())
TEST(set, iterator_based_ops) {
    const std::set<int> s = {1, 2, 3, 4, 5};
    assert(ranged::any(s, [](const int &x) { return x == 3; }));
    assert(ranged::all(s, [](const int &x) { return x >= 1; }));
    assert(ranged::contains(s, 5));
    assert(ranged::count_if(s, [](const int &x) { return x % 2 == 1; }) == 3);
    const int first_gt_2 = ranged::first_or_default(s, [](const int &x) { return x > 2; });
    assert(first_gt_2 == 3);
    const auto filtered = ranged::filter(s, [](const int &x) { return x >= 4; });
    assert(filtered.size() == 2 && filtered.find(4) != filtered.end() && filtered.find(5) != filtered.end());
}

TEST(set, select_test) {
    const std::set<int> s = {1, 2, 3, 4, 5};
    const std::set<std::string> expected = {"1", "2", "3", "4", "5"};
    const auto result = ranged::select(s, [](const int &i) {return std::to_string(i);});
    assert(result == expected);
}

TEST(unordered_set, iterator_based_ops) {
    const std::unordered_set<int> s = {1, 2, 3, 4, 5};
    assert(ranged::any(s, [](const int &x) { return x == 2; }));
    assert(ranged::contains(s, 5));
    assert(ranged::count_if(s, [](const int &x) { return x > 3; }) == 2);
    const auto filtered = ranged::filter(s, [](const int &x) { return x < 3; });
    assert(filtered.size() == 2 && filtered.find(1) != filtered.end() && filtered.find(2) != filtered.end());
}

TEST(unordered_set, select_test) {
    const std::unordered_set<int> s = {1, 2, 3, 4, 5};
    const auto result = ranged::select(s, [](const int &i) {return i * 2;});
    assert(result.size() == 5);
    assert(result.find(2) != result.end());
    assert(result.find(4) != result.end());
    assert(result.find(6) != result.end());
    assert(result.find(8) != result.end());
    assert(result.find(10) != result.end());
}

// map tests (operate on value_type = pair<const K,V>)
TEST(map, iterator_based_ops) {
    const std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    assert(ranged::any(m, [](const std::pair<const std::string, int> &p) { return p.second == 2; }));
    assert(ranged::all(m, [](const std::pair<const std::string, int> &p) { return p.second >= 1; }));
    assert(ranged::count_if(m, [](const std::pair<const std::string, int> &p) { return p.second % 2 == 1; }) == 2);
    const auto first_two = ranged::first_or_default(m, [](const std::pair<const std::string, int> &p) { return p.second == 2; });
    assert(first_two.second == 2 && first_two.first == "b");
    const auto filtered = ranged::filter(m, [](const std::pair<const std::string, int> &p) { return p.second >= 2; });
    assert(filtered.size() == 2 && filtered.find("b") != filtered.end() && filtered.find("c") != filtered.end());
}

// TEST(map, select_test) {
//     const std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
//     const auto result = ranged::select(m, [](const std::pair<const std::string, int> &p) {
//         return std::make_pair<std::string, int>(std::string(p.first), p.second * 10);
//     });
//     assert(result.size() == 3);
//     assert(result.find(10) != result.end());
//     assert(result.find(20) != result.end());
//     assert(result.find(30) != result.end());
// }

TEST(unordered_map, iterator_based_ops) {
    const std::unordered_map<std::string, int> m = {{"x", 10}, {"y", 20}, {"z", 30}};
    assert(ranged::any(m, [](const std::pair<const std::string, int> &p) { return p.second == 20; }));
    assert(ranged::count_if(m, [](const std::pair<const std::string, int> &p) { return p.second >= 20; }) == 2);
    const auto filtered = ranged::filter(m, [](const std::pair<const std::string, int> &p) { return p.second > 10; });
    assert(filtered.size() == 2 && filtered.find("y") != filtered.end() && filtered.find("z") != filtered.end());
}

// TEST(unordered_map, select_test) {
//     const std::unordered_map<std::string, int> m = {{"x", 10}, {"y", 20}, {"z", 30}};
//     const auto result = ranged::select(m, [](const std::pair<const std::string, int> &p) {return p.first + "_key";});
//     assert(result.size() == 3);
//     assert(result.find("x_key") != result.end());
//     assert(result.find("y_key") != result.end());
//     assert(result.find("z_key") != result.end());
// }

// list tests (iterator-based only)
TEST(list, iterator_based_ops) {
    const std::list<int> l = {1, 2, 3, 4, 5};
    assert(ranged::any(l, [](const int &x) { return x == 5; }));
    assert(!ranged::all(l, [](const int &x) { return x < 5; }));
    assert(ranged::count_if(l, [](const int &x) { return x > 2; }) == 3);
    const int first = ranged::first_or_default(l, [](const int &x) { return x > 3; });
    assert(first == 4);
    const auto filtered = ranged::filter(l, [](const int &x) { return x % 2 == 0; });
    assert(filtered.size() == 2);
}

TEST(list, select_test) {
    const std::list<int> l = {1, 2, 3, 4, 5};
    const std::list<std::string> expected = {"1", "2", "3", "4", "5"};
    const auto result = ranged::select(l, [](const int &i) {return std::to_string(i);});
    assert(result == expected);
}

int main() {
    dispatcher::run_tests<std::chrono::nanoseconds>();
    return 0;
}
