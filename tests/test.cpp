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
    assert(ranged::to<std::vector>(v) == v);
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
    const auto result = ranged::to<std::vector>(ranged::filter(v, [](const int &i) { return i > 5; }));
    assert(result == expected);
}

TEST(vector, select_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const std::vector<std::string> expected = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    const auto result = ranged::to<std::vector>(ranged::transform(v, [](const int &i) {return std::to_string(i);}));
    assert(result == expected);
}

TEST(vector, zip_test) {
    std::vector<int> v1 = {1, 2, 3, 4, 5};
    std::vector<int> v2 = {10, 20, 30, 40, 50};
    const auto result = ranged::to<std::vector>(ranged::zip(v1, v2));
    assert(result.size() == 5);
    assert(std::get<0>(result[0]) == 1 && std::get<1>(result[0]) == 10);
    assert(std::get<0>(result[2]) == 3 && std::get<1>(result[2]) == 30);
    assert(std::get<0>(result[4]) == 5 && std::get<1>(result[4]) == 50);
}

TEST(vector, to_list_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const auto result = ranged::to<std::list>(v);
    assert(result.size() == 10);
    assert(ranged::to<std::vector>(result) == v);
}

TEST(vector, to_set_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const auto result = ranged::to<std::set>(v);
    assert(result.size() == 10);
    assert(ranged::to<std::vector>(result) == v);
}

TEST(vector, to_unordered_set_test) {
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const auto result = ranged::to<std::unordered_set>(v);
    assert(result.size() == 10);
}

TEST(deque, any_test) {
    const std::deque<int> d = {1, 2, 3, 4, 5, 6};
    assert(ranged::any(d, [](const int &x) { return x == 4; }));
}

TEST(deque, dummy_vector_all_test) {
    const std::deque<int> d = {1, 2, 3, 4, 5, 6};
    assert(ranged::all(d, [](const int &x) { return x >= 1; }));
}

TEST(deque, contains_test) {
    const std::deque<int> d = {1, 2, 3, 4, 5, 6};
    assert(ranged::contains(d, 6));
}

TEST(deque, count_if_counts_even_numbers) {
    const std::deque<int> d = {1, 2, 3, 4, 5, 6};
    assert(ranged::count_if(d, [](const int &x) { return x % 2 == 0; }) == 3);
}

TEST(deque, first_or_default_test) {
    const std::deque<int> d = {1, 2, 3, 4, 5, 6};
    const int first_over_3 = ranged::first_or_default(d, [](const int &x) { return x > 3; });
    assert(first_over_3 == 4);
}

TEST(deque, to_array) {
    const std::deque<int> d = {5, 4, 9, 1, 3};
    const auto a = ranged::to_array<5>(d);
    assert(a.size() == 5);
    assert(a[2] == 9);
}

TEST(deque, max) {
    const std::deque<int> d = {5, 4, 9, 1, 3};
    assert(ranged::max(d) == 9);
}

TEST(deque, min) {
    const std::deque<int> d = {9, 7, 5, 3, 2};
    assert(ranged::min(d) == 2);
}

TEST(deque, to_vector) {
    const std::deque<int> d = {5, 4, 9, 1, 3};
    const auto v = ranged::to<std::vector>(d);
    assert(v.size() == 5);
    assert(v[0] == 5);
    assert(v[4] == 3);
}

TEST(deque, filter_test) {
    const std::deque<int> d = {1, 2, 3, 4, 5, 6};
    const auto res = ranged::to<std::vector>(ranged::filter(d, std::function<bool(const int &)>([](const int &x) { return x > 3; })));
    assert(res.size() == 3);
    assert(res.at(0) == 4 && res.at(2) == 6);
}

TEST(deque, select_test) {
    const std::deque<int> d = {1, 2, 3, 4, 5};
    const std::deque<std::string> expected = {"1", "2", "3", "4", "5"};
    const auto result = ranged::to<std::deque>(ranged::transform(d, std::function<std::string(const int &)>([](const int &i) {return std::to_string(i);})));
    assert(result == expected);
}

TEST(deque, zip_test) {
    std::deque<int> d1 = {1, 2, 3};
    std::deque<int> d2 = {10, 20, 30};
    const auto result = ranged::to<std::vector>(ranged::zip(d1, d2));
    assert(result.size() == 3);
    assert(std::get<0>(result[0]) == 1 && std::get<1>(result[0]) == 10);
    assert(std::get<0>(result[1]) == 2 && std::get<1>(result[1]) == 20);
    assert(std::get<0>(result[2]) == 3 && std::get<1>(result[2]) == 30);
}

// array tests (only functions explicitly overloaded or iterator-based where constraints allow)
TEST(array, filter_test) {
    const std::array<int, 6> a = {{1, 2, 3, 4, 5, 6}};
    const std::vector<int> expected = {4, 5, 6};
    const auto res = ranged::filter<int, 6>(a, [](const int &x) { return x > 3; });
    assert(res == expected);
}

TEST(array, zip_test) {
    std::array<int, 4> a1 = {1, 2, 3, 4};
    std::array<int, 4> a2 = {10, 20, 30, 40};
    const auto result = ranged::zip(a1, a2);
    assert(result.size() == 4);
    assert(std::get<0>(result[0]) == 1 && std::get<1>(result[0]) == 10);
    assert(std::get<0>(result[1]) == 2 && std::get<1>(result[1]) == 20);
    assert(std::get<0>(result[3]) == 4 && std::get<1>(result[3]) == 40);
}

// set tests (works for iterator-based ops; avoid max/min/to_array which need at())
TEST(set, any_test) {
    const std::set<int> s = {1, 2, 3, 4, 5};
    assert(ranged::any(s, [](const int &x) { return x == 3; }));
}

TEST(set, dummy_vector_all_test) {
    const std::set<int> s = {1, 2, 3, 4, 5};
    assert(ranged::all(s, [](const int &x) { return x >= 1; }));
}

TEST(set, contains_test) {
    const std::set<int> s = {1, 2, 3, 4, 5};
    assert(ranged::contains(s, 5));
}

TEST(set, dummy_vector_count_if_test) {
    const std::set<int> s = {1, 2, 3, 4, 5};
    assert(ranged::count_if(s, [](const int &x) { return x % 2 == 1; }) == 3);
}

TEST(set, first_or_default_test) {
    const std::set<int> s = {1, 2, 3, 4, 5};
    const int first_gt_2 = ranged::first_or_default(s, [](const int &x) { return x > 2; });
    assert(first_gt_2 == 3);
}

TEST(set, filter_test) {
    const std::set<int> s = {1, 2, 3, 4, 5};
    const auto filtered = ranged::to<std::set>(ranged::filter(s, [](const int &x) { return x >= 4; }));
    assert(filtered.size() == 2);
    assert(filtered.find(4) != filtered.end());
    assert(filtered.find(5) != filtered.end());
}

TEST(set, select_test) {
    const std::set<int> s = {1, 2, 3, 4, 5};
    const std::set<std::string> expected = {"1", "2", "3", "4", "5"};
    const auto result = ranged::to<std::set>(ranged::transform(s, [](const int &i) {return std::to_string(i);}));
    assert(result == expected);
}

TEST(set, zip_test) {
    const std::set<int> s = {1, 2, 3, 4, 5};
    const std::set<int> s2 = {1, 2, 3, 4, 5};
    const auto result = ranged::to<std::vector>(ranged::zip(s, s2));
    assert(result.size() == 5);
}
TEST(unordered_set, any_test) {
    const std::unordered_set<int> s = {1, 2, 3, 4, 5};
    assert(ranged::any(s, [](const int &x) { return x == 2; }));
}

TEST(unordered_set, contains_test) {
    const std::unordered_set<int> s = {1, 2, 3, 4, 5};
    assert(ranged::contains(s, 5));
}

TEST(unordered_set, dummy_vector_count_if_test) {
    const std::unordered_set<int> s = {1, 2, 3, 4, 5};
    assert(ranged::count_if(s, [](const int &x) { return x > 3; }) == 2);
}

TEST(unordered_set, filter_test) {
    const std::unordered_set<int> s = {1, 2, 3, 4, 5};
    const auto filtered = ranged::to<std::unordered_set>(ranged::filter(s, [](const int &x) { return x < 3; }));
    assert(filtered.size() == 2);
    assert(filtered.find(1) != filtered.end());
    assert(filtered.find(2) != filtered.end());
}

TEST(unordered_set, select_test) {
    const std::unordered_set<int> s = {1, 2, 3, 4, 5};
    const auto result = ranged::to<std::unordered_set>(ranged::transform(s, [](const int &i) {return i * 2;}));
    assert(result.size() == 5);
    assert(result.find(2) != result.end());
    assert(result.find(4) != result.end());
    assert(result.find(6) != result.end());
    assert(result.find(8) != result.end());
    assert(result.find(10) != result.end());
}

TEST(unordered_set, zip_test) {
    std::unordered_set<int> s1 = {1, 2, 3};
    std::unordered_set<int> s2 = {10, 20, 30};
    const auto result = ranged::to<std::set>(ranged::zip(s1, s2));
    assert(result.size() == 3);
    // For unordered_set, we can't rely on order, so check that all pairs exist
    bool found_1_10 = false, found_2_20 = false, found_3_30 = false;
    for (const auto &p : result) {
        if (std::get<0>(p) == 1 && std::get<1>(p) == 10) found_1_10 = true;
        if (std::get<0>(p) == 2 && std::get<1>(p) == 20) found_2_20 = true;
        if (std::get<0>(p) == 3 && std::get<1>(p) == 30) found_3_30 = true;
    }
    assert(found_1_10 && found_2_20 && found_3_30);
}

// map tests (operate on value_type = pair<const K,V>)
TEST(map, any_test) {
    const std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    assert(ranged::any(m, [](const std::pair<const std::string, int> &p) { return p.second == 2; }));
}

TEST(map, dummy_vector_all_test) {
    const std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    assert(ranged::all(m, [](const std::pair<const std::string, int> &p) { return p.second >= 1; }));
}

TEST(map, dummy_vector_count_if_test) {
    const std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    assert(ranged::count_if(m, [](const std::pair<const std::string, int> &p) { return p.second % 2 == 1; }) == 2);
}

TEST(map, first_or_default_test) {
    const std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    const auto first_two = ranged::first_or_default(m, [](const std::pair<const std::string, int> &p) { return p.second == 2; });
    assert(first_two.second == 2);
    assert(first_two.first == "b");
}

TEST(map, filter_test) {
    const std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    const auto filtered = ranged::to<std::map>(ranged::filter(m, [](const std::pair<const std::string, int> &p) { return p.second >= 2; }));
    assert(filtered.size() == 2);
    assert(filtered.find("b") != filtered.end());
    assert(filtered.find("c") != filtered.end());
}

TEST(map, select_test) {
    const std::map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
    const auto result = ranged::to<std::map>(ranged::transform(m, [](const std::pair<const std::string, int> &p) {
        return std::make_pair<std::string, std::string>(std::string(p.first), std::to_string(p.second * 10));
    }));
    assert(result.size() == 3);
    assert(result.at("a") == "10");
    assert(result.at("b") == "20");
    assert(result.at("c") == "30");
}

TEST(unordered_map, any_test) {
    const std::unordered_map<std::string, int> m = {{"x", 10}, {"y", 20}, {"z", 30}};
    assert(ranged::any(m, [](const std::pair<const std::string, int> &p) { return p.second == 20; }));
}

TEST(unordered_map, dummy_vector_count_if_test) {
    const std::unordered_map<std::string, int> m = {{"x", 10}, {"y", 20}, {"z", 30}};
    assert(ranged::count_if(m, [](const std::pair<const std::string, int> &p) { return p.second >= 20; }) == 2);
}

TEST(unordered_map, filter_test) {
    const std::unordered_map<std::string, int> m = {{"x", 10}, {"y", 20}, {"z", 30}};
    const auto filtered = ranged::to<std::unordered_map>(ranged::filter(m, [](const std::pair<const std::string, int> &p) { return p.second > 10; }));
    assert(filtered.size() == 2);
    assert(filtered.find("y") != filtered.end());
    assert(filtered.find("z") != filtered.end());
}

TEST(unordered_map, select_test) {
    const std::unordered_map<std::string, int> m = {{"x", 10}, {"y", 20}, {"z", 30}};
    const auto result = ranged::to<std::unordered_map>(ranged::transform(m, [](const std::pair<const std::string, int> &p) {
        return std::make_pair<std::string, char>(std::string(p.first + "_key"), p.second);
    }));
    assert(result.size() == 3);
    assert(result.find("x_key") != result.end());
    assert(result.find("y_key") != result.end());
    assert(result.find("z_key") != result.end());
}

// list tests (iterator-based only)
TEST(list, any_test) {
    const std::list<int> l = {1, 2, 3, 4, 5};
    assert(ranged::any(l, [](const int &x) { return x == 5; }));
}

TEST(list, all_returns_false_when_predicate_fails) {
    const std::list<int> l = {1, 2, 3, 4, 5};
    assert(!ranged::all(l, [](const int &x) { return x < 5; }));
}

TEST(list, dummy_vector_count_if_test) {
    const std::list<int> l = {1, 2, 3, 4, 5};
    assert(ranged::count_if(l, [](const int &x) { return x > 2; }) == 3);
}

TEST(list, first_or_default_test) {
    const std::list<int> l = {1, 2, 3, 4, 5};
    const int first = ranged::first_or_default(l, [](const int &x) { return x > 3; });
    assert(first == 4);
}

TEST(list, filter_test) {
    const std::list<int> l = {1, 2, 3, 4, 5};
    const auto filtered = ranged::to<std::list>(ranged::filter(l, [](const int &x) { return x % 2 == 0; }));
    assert(filtered.size() == 2);
}

TEST(list, select_test) {
    const std::list<int> l = {1, 2, 3, 4, 5};
    const std::list<std::string> expected = {"1", "2", "3", "4", "5"};
    const auto result = ranged::to<std::list>(ranged::transform(l, [](const int &i) {return std::to_string(i);}));
    assert(result == expected);
}

TEST(list, zip_test) {
    std::list<std::string> l1 = {"1", "2", "3"};
    std::list<int> l2 = {10, 20, 30};
    const auto result = ranged::to<std::list>(ranged::zip(l1, l2));
    assert(result.size() == 3);
    // For list, iteration order is preserved
    auto it = result.begin();
    assert(std::get<0>(*it) == "1" && std::get<1>(*it) == 10);
    ++it;
    assert(std::get<0>(*it) == "2" && std::get<1>(*it) == 20);
    ++it;
    assert(std::get<0>(*it) == "3" && std::get<1>(*it) == 30);
}

std::string create_string() { return "temporary"; }

int main() {
    dispatcher::run_tests<std::chrono::nanoseconds>();
    return 0;
}
