//
// Created by mmatz on 9/3/25.
//

#ifndef RANGED_H
#define RANGED_H
#include <algorithm>
#include <cstddef>
#include <list>
#include <utility>
#include <vector>

#ifndef RANGED_NO_DEPRECATION_WARNINGS
#define RANGED_NO_DEPRECATION_WARNINGS 0
#endif

namespace ranged {

#if __cplusplus >= 202002L
    template<typename T>
    concept _std_container_ = requires(T a) {
        typename T::value_type;
        typename T::allocator_type;
        typename T::size_type;
        typename T::iterator;
        typename T::const_iterator;
    };
#else
#define _std_container_ typename
#endif

    namespace _decl {

        template<class T>
        struct more;

        template<class T = void>
        struct more {
            constexpr bool operator()(const T &lhs, const T &rhs) const { return lhs > rhs; };
        };

        template<typename Iter, typename Pred>
        class conditional_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = typename std::iterator_traits<Iter>::value_type;
            using difference_type = typename std::iterator_traits<Iter>::difference_type;
            using pointer = typename std::iterator_traits<Iter>::pointer;
            using reference = typename std::iterator_traits<Iter>::reference;
#if __cplusplus >= 201703L
            using base_iterator = std::remove_const_t<std::remove_reference_t<Iter>>;
#else
            using base_iterator = typename std::remove_const<typename std::remove_reference<Iter>::type>::type;
#endif

            conditional_iterator() = default;

            template<typename I1, typename I2>
            conditional_iterator(I1 &&begin, I2 &&end, Pred pred) noexcept :
                current_(std::forward<I1>(begin)), end_(std::forward<I2>(end)), pred_(pred) {
                satisfy();
            }

            constexpr reference operator*() const noexcept { return *current_; }
            constexpr pointer operator->() const noexcept { return &(*current_); }

            conditional_iterator &operator++() {
                ++current_;
                satisfy();
                return *this;
            }

            conditional_iterator operator++(int) {
                conditional_iterator tmp = *this;
                ++*this;
                return tmp;
            }

            constexpr friend bool operator==(const conditional_iterator &lhs, const conditional_iterator &rhs) {
                return lhs.current_ == rhs.current_;
            }

            constexpr friend bool operator!=(const conditional_iterator &lhs, const conditional_iterator &rhs) {
                return lhs.current_ != rhs.current_;
            }

        private:
            void satisfy() {
                while (current_ != end_ && !pred_(*current_)) {
                    ++current_;
                }
            }

            base_iterator current_{};
            base_iterator end_{};
            Pred pred_;
        };
        template<typename Range, typename Pred>
        class conditional_range {
        public:
            using IteratorType = decltype(std::begin(std::declval<Range &>()));
            using iterator = conditional_iterator<IteratorType, Pred>;

            conditional_range(Range &range, Pred pred) :
                begin_it(std::begin(range), std::end(range), pred), end_it(std::end(range), std::end(range), pred) {}

            iterator begin() { return begin_it; }
            iterator end() { return end_it; }
            iterator begin() const { return begin_it; }
            iterator end() const { return end_it; }

        private:
            iterator begin_it;
            iterator end_it;
        };


        template<typename Iter, typename Pred>
        class projectional_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
#if __cplusplus >= 201703L
            using value_type = std::result_of_t<Pred(typename std::iterator_traits<Iter>::value_type)>;
#else
            using value_type = typename std::result_of<Pred(typename std::iterator_traits<Iter>::value_type)>::type;
#endif
            using difference_type = typename std::iterator_traits<Iter>::difference_type;
            using pointer = value_type *;
            using reference = value_type &;
#if __cplusplus >= 201703L
            using ibase_iterator = std::remove_const_t<std::remove_reference_t<Iter>>;
#else
            using ibase_iterator = typename std::remove_const<typename std::remove_reference<Iter>::type>::type;
#endif

            projectional_iterator() = default;

            template<typename I1, typename I2>
            projectional_iterator(I1 &&begin, I2 &&end, Pred pred) noexcept :
                current_(std::forward<I1>(begin)), end_(std::forward<I2>(end)), pred_(pred) {}

            constexpr value_type operator*() const noexcept { return pred_(*current_); }
            constexpr pointer operator->() const noexcept { return &pred_(*current_); }

            projectional_iterator &operator++() {
                ++current_;
                return *this;
            }

            projectional_iterator operator++(int) {
                projectional_iterator tmp = *this;
                ++*this;
                return tmp;
            }

            constexpr friend bool operator==(const projectional_iterator &lhs, const projectional_iterator &rhs) {
                return lhs.current_ == rhs.current_;
            }

            constexpr friend bool operator!=(const projectional_iterator &lhs, const projectional_iterator &rhs) {
                return lhs.current_ != rhs.current_;
            }

        private:
            ibase_iterator current_{};
            ibase_iterator end_{};
            Pred pred_;
        };
        template<typename Range, typename Pred>
        class projection_range {
        public:
            using IteratorType = decltype(std::begin(std::declval<Range &>()));
            using iterator = projectional_iterator<IteratorType, Pred>;

            projection_range(Range &range, Pred pred) :
                begin_it(std::begin(range), std::end(range), pred), end_it(std::end(range), std::end(range), pred) {}

            iterator begin() { return begin_it; }
            iterator end() { return end_it; }
            iterator begin() const { return begin_it; }
            iterator end() const { return end_it; }

        private:
            iterator begin_it;
            iterator end_it;
        };

    } // namespace _decl

    template<_std_container_ T, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::any_of` instead")]]
#endif
    constexpr bool any(const T &container, const Pred &func);
    template<_std_container_ T, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::any_of` instead")]]
#endif
    constexpr bool any(T &container, const Pred &func);
    template<_std_container_ T, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::all_of` instead")]]
#endif
    constexpr bool all(const T &container, const Pred &func);
    template<_std_container_ T, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::all_of` instead")]]
#endif
    constexpr bool all(T &container, const Pred &func);
    template<_std_container_ T>
    constexpr bool contains(const T &container, const typename T::value_type &value);
    template<_std_container_ T>
    constexpr bool contains(T &container, const typename T::value_type &value);
    template<_std_container_ T, typename Func>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::for_each` instead")]]
#endif
    constexpr void for_each(const T &container, const Func &func);
    template<_std_container_ T, typename Func>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::for_each` instead")]]
#endif
    constexpr void for_each(T &container, const Func &func);
    template<_std_container_ T, typename Pred>
    constexpr typename T::value_type
    first_or_default(const T &container, const Pred &func,
                     const typename T::value_type &default_value = typename T::value_type());
    template<_std_container_ T, typename Pred>
    constexpr typename T::value_type
    first_or_default(T &container, const Pred &func,
                     const typename T::value_type &default_value = typename T::value_type());

    template<_std_container_ T, class AllocT = std::allocator<typename T::value_type>>
    constexpr std::vector<typename T::value_type, AllocT> to_vector(const T &container);
    template<_std_container_ T, class AllocT = std::allocator<typename T::value_type>>
    constexpr std::vector<typename T::value_type, AllocT> to_vector(T &container);
    template<size_t N, _std_container_ T>
    constexpr std::array<typename T::value_type, N> to_array(const T &container);
    template<size_t N, _std_container_ T>
    constexpr std::array<typename T::value_type, N> to_array(T &container);
    template<_std_container_ T, class AllocT = std::allocator<typename T::value_type>>
    constexpr std::deque<typename T::value_type, AllocT> to_deque(const T &container);
    template<_std_container_ T, class AllocT = std::allocator<typename T::value_type>>
    constexpr std::deque<typename T::value_type, AllocT> to_deque(T &container);
    template<_std_container_ T, class AllocT = std::allocator<typename T::value_type>>
    constexpr std::list<typename T::value_type, AllocT> to_list(const T &container);
    template<_std_container_ T, class AllocT = std::allocator<typename T::value_type>>
    constexpr std::list<typename T::value_type, AllocT> to_list(T &container);
    template<_std_container_ T, class Compare = std::less<typename T::value_type>, class AllocT = std::allocator<typename T::value_type>>
    constexpr std::set<typename T::value_type, Compare, AllocT> to_set(const T &container);
    template<_std_container_ T, class Compare = std::less<typename T::value_type>, class AllocT = std::allocator<typename T::value_type>>
    constexpr std::set<typename T::value_type, Compare, AllocT> to_set(T &container);
    template<_std_container_ T, class Hash = std::hash<typename T::value_type>, class KeyEqual = std::equal_to<typename T::value_type>, class AllocT = std::allocator<typename T::value_type>>
    constexpr std::unordered_set<typename T::value_type, Hash, KeyEqual, AllocT> to_unordered_set(const T &container);
    template<_std_container_ T, class Hash = std::hash<typename T::value_type>, class KeyEqual = std::equal_to<typename T::value_type>, class AllocT = std::allocator<typename T::value_type>>
    constexpr std::unordered_set<typename T::value_type, Hash, KeyEqual, AllocT> to_unordered_set(T &container);
    template<_std_container_ T, typename Pred>
    constexpr size_t count_if(const T &container, const Pred &func);
    template<_std_container_ T, typename Pred>
    constexpr size_t count_if(T &container, const Pred &func);
    template<_std_container_ T, typename Compare = std::less<typename T::value_type>>
    constexpr typename T::value_type max(const T &container, const Compare &cmp = {});
    template<_std_container_ T, typename Compare = std::less<typename T::value_type>>
    constexpr typename T::value_type max(T &container, const Compare &cmp = {});
    template<_std_container_ T, typename Compare = _decl::more<typename T::value_type>>
    constexpr typename T::value_type min(const T &container, const Compare &cmp = {});
    template<_std_container_ T, typename Compare = _decl::more<typename T::value_type>>
    constexpr typename T::value_type min(T &container, const Compare &cmp = {});

    template<_std_container_ T, typename Pred>
    constexpr T filter(T &container, Pred func);
    template<_std_container_ T, typename Pred>
    constexpr T filter(const T &container, Pred func);
    template<typename T, size_t N, typename Pred, class AllocT = std::allocator<T>>
    constexpr std::vector<T> filter(const std::array<T, N> &array, Pred pred);

    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr T<typename std::result_of<Pred(typename T<Args...>::value_type)>::type> select(T<Args...> &container,
                                                                                             Pred pred);
    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr T<typename std::result_of<Pred(typename T<Args...>::value_type)>::type>
    select(const T<Args...> &container, Pred pred);
    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr T<typename std::result_of<Pred(typename T<Args...>::value_type)>::type::first_type,
                typename std::result_of<Pred(typename T<Args...>::value_type)>::type::second_type>
    select(T<Args...> &container, Pred pred);
    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr T<typename std::result_of<Pred(typename T<Args...>::value_type)>::type::first_type,
                typename std::result_of<Pred(typename T<Args...>::value_type)>::type::second_type>
    select(const T<Args...> &container, Pred pred);

    template<template<typename, typename> class T, typename T1, typename T2, typename Alloc1, typename Alloc2,
             typename AllocResult = std::allocator<std::pair<T1, T2>>>
    constexpr T<std::pair<T1, T2>, AllocResult> zip(T<T1, Alloc1> &first, T<T2, Alloc2> &second);
    template<template<typename, typename> class T, typename T1, typename T2, typename Alloc1, typename Alloc2,
             typename AllocResult = std::allocator<std::pair<T1, T2>>>
    constexpr T<std::pair<T1, T2>, AllocResult> zip(const T<T1, Alloc1> &first, const T<T2, Alloc2> &second);

    template<template<typename, std::size_t> class Ta, typename value_type, std::size_t N>
    constexpr Ta<std::pair<value_type, value_type>, N> zip(Ta<value_type, N> &first, Ta<value_type, N> &second);
    template<template<typename, std::size_t> class Ta, typename value_type, std::size_t N>
    constexpr Ta<std::pair<value_type, value_type>, N> zip(const Ta<value_type, N> &first,
                                                            const Ta<value_type, N> &second);

    // make `zip` for types without indexer (e.g. std::list, std::set, etc.)


#ifdef RANGED_IMPLEMENTATION


    template<_std_container_ T, typename Cond>
    constexpr bool any(const T &container, const Cond &func) {
#if __cplusplus >= 202002L
        return std::ranges::any_of(container, func);
#else
        for (const auto &element: container) {
            if (func(element))
                return true;
        }

        return false;
#endif
    }
    template<_std_container_ T, typename Pred>
    constexpr bool any(T &container, const Pred &func) {
#if __cplusplus >= 202002L
        return std::ranges::any_of(container, func);
#else
        for (const auto &element: container) {
            if (func(element))
                return true;
        }

        return false;
#endif
    }

    template<_std_container_ T, typename Cond>
    constexpr bool all(const T &container, const Cond &func) {
#if __cplusplus >= 202002L
        return std::ranges::all_of(container, func);
#else
        for (const auto &element: container) {
            if (!func(element))
                return false;
        }

        return true;
#endif
    }
    template<_std_container_ T, typename Pred>
    constexpr bool all(T &container, const Pred &func) {
#if __cplusplus >= 202002L
        return std::ranges::all_of(container, func);
#else
        for (const auto &element: container) {
            if (!func(element))
                return false;
        }

        return true;
#endif
    }
    template<_std_container_ T>
    constexpr bool contains(const T &container, const typename T::value_type &value) {
#if __cplusplus >= 202002L
        return std::ranges::find(container, value) != container.end();
#else
        return std::find(container.begin(), container.end(), value) != container.end();
#endif
    }
    template<_std_container_ T>
    constexpr bool contains(T &container, const typename T::value_type &value) {
#if __cplusplus >= 202002L
        return std::ranges::find(container, value) != container.end();
#else
        return std::find(container.begin(), container.end(), value) != container.end();
#endif
    }
    template<_std_container_ T, typename Func>
    constexpr void for_each(const T &container, const Func &func) {
#if __cplusplus >= 202002L
        std::ranges::for_each(container, func);
#else
        for (const auto &element: container) {
            func(element);
        }
#endif
    }
    template<_std_container_ T, typename Func>
    constexpr void for_each(T &container, const Func &func) {
#if __cplusplus >= 202002L
        std::ranges::for_each(container, func);
#else
        for (const auto &element: container) {
            func(element);
        }
#endif
    }
    template<_std_container_ T, typename Cond>
    constexpr auto first_or_default(const T &container, const Cond &func, const typename T::value_type &default_value)
            -> typename T::value_type {
        for (const auto &item: container) {
            if (func(item))
                return item;
        }

        return default_value;
    }
    template<_std_container_ T, typename Pred>
    constexpr typename T::value_type first_or_default(T &container, const Pred &func,
                                                      const typename T::value_type &default_value) {
        for (const auto &item: container) {
            if (func(item))
                return item;
        }

        return default_value;
    }
    template<_std_container_ T, class AllocT>
    constexpr auto to_vector(const T &container) -> std::vector<typename T::value_type, AllocT> {
        std::vector<typename T::value_type, AllocT> result{container.begin(), container.end()};
        return result;
    }
    template<_std_container_ T, class AllocT>
    constexpr std::vector<typename T::value_type, AllocT> to_vector(T &container) {
        std::vector<typename T::value_type, AllocT> result{container.begin(), container.end()};
        return result;
    }
    template<size_t N, _std_container_ T>
    constexpr auto to_array(const T &container) -> std::array<typename T::value_type, N> {
        std::array<typename T::value_type, N> result{};
        const size_t iter_to = std::min(N, container.size());
        for (size_t i{0}; i < iter_to; ++i) {
            result[i] = container.at(i);
        }

        return result;
    }
    template<size_t N, _std_container_ T>
    constexpr std::array<typename T::value_type, N> to_array(T &container) {
        std::array<typename T::value_type, N> result{};
        const size_t iter_to = std::min(N, container.size());
        for (size_t i{0}; i < iter_to; ++i) {
            result[i] = container.at(i);
        }

        return result;
    }
    template<_std_container_ T, class AllocT>
    constexpr std::deque<typename T::value_type, AllocT> to_deque(const T &container) {
        using value_type = typename T::value_type;
        std::deque<value_type, AllocT> result{container.begin(), container.end()};
        return result;
    }
    template<_std_container_ T, class AllocT>
    constexpr std::deque<typename T::value_type, AllocT> to_deque(T &container) {
        using value_type = typename T::value_type;
        std::deque<value_type, AllocT> result{container.begin(), container.end()};
        return result;
    }
    template<_std_container_ T, class AllocT>
    constexpr std::list<typename T::value_type, AllocT> to_list(const T &container) {
        using value_type = typename T::value_type;
        std::list<value_type, AllocT> result{container.begin(), container.end()};
        return result;
    }
    template<_std_container_ T, class AllocT>
    constexpr std::list<typename T::value_type, AllocT> to_list(T &container) {
        using value_type = typename T::value_type;
        std::list<value_type, AllocT> result{container.begin(), container.end()};
        return result;
    }
    template<_std_container_ T, class Compare, class AllocT>
    constexpr std::set<typename T::value_type, Compare, AllocT> to_set(const T &container) {
        using key_type = typename T::value_type;
        std::set<key_type, Compare, AllocT> result{container.begin(), container.end()};
        return result;
    }
    template<_std_container_ T, class Compare, class AllocT>
    constexpr std::set<typename T::value_type, Compare, AllocT> to_set(T &container) {
        using key_type = typename T::value_type;
        std::set<key_type, Compare, AllocT> result{container.begin(), container.end()};
        return result;
    }
    template<_std_container_ T, class Hash, class KeyEqual, class AllocT>
    constexpr std::unordered_set<typename T::value_type, Hash, KeyEqual, AllocT> to_unordered_set(const T &container) {
        using key_type = typename T::value_type;
        std::unordered_set<key_type, Hash, KeyEqual, AllocT> result{container.begin(), container.end()};
        return result;
    }
    template<_std_container_ T, class Hash, class KeyEqual, class AllocT>
    constexpr std::unordered_set<typename T::value_type, Hash, KeyEqual, AllocT> to_unordered_set(T &container) {
        using key_type = typename T::value_type;
        std::unordered_set<key_type, Hash, KeyEqual, AllocT> result{container.begin(), container.end()};
        return result;
    }
    template<_std_container_ T, size_t N>
    constexpr std::array<typename T::value_type, N> to_array(T &container) {
        std::array<typename T::value_type, N> result{};
        const size_t iter_to = std::min(N, container.size());
        for (size_t i{0}; i < iter_to; ++i) {
            result[i] = container.at(i);
        }

        return result;
    }
    template<_std_container_ T, typename Cond>
    constexpr size_t count_if(const T &container, const Cond &func) {
        size_t result{};
        for (const auto &item: container) {
            if (func(item))
                ++result;
        }

        return result;
    }
    template<_std_container_ T, typename Pred>
    constexpr size_t count_if(T &container, const Pred &func) {
        size_t result{};
        for (const auto &item: container) {
            if (func(item))
                ++result;
        }

        return result;
    }
    template<_std_container_ T, typename Cond>
    constexpr T filter(T &container, Cond func) {
        const auto &iter = _decl::conditional_range<T, Cond>{container, func};
        return T{iter.begin(), iter.end()};
    }

    template<_std_container_ T, typename Cond>
    constexpr T filter(const T &container, Cond func) {
        const auto &iter = _decl::conditional_range<const T, Cond>{container, func};
        return T{iter.begin(), iter.end()};
    }
    template<typename  T, size_t N, typename Pred, class AllocT>
    constexpr std::vector<T> filter(const std::array<T, N> &array, Pred pred) {
        std::vector<T, AllocT> result(N);
#if __cplusplus >= 202002L
        auto [in_it, out_it] = std::ranges::copy_if(array, result.begin(), pred);
        result.resize(std::distance(result.begin(), out_it));
#elif __cplusplus >= 201402L
        size_t i{0};
        for (size_t j{0}; j < N; ++j) {
            if (pred(array.at(j))) {
                result[i] = array.at(j);
                ++i;
            }
        }
        result.resize(i);
#else
        size_t i{0};
        for (size_t j{0}; j < N; ++j) {
            if (pred(array.at(j)))
                result[i++] = array.at(j);
        }
        result.resize(i);
#endif
        return result;
    }

    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr T<typename std::result_of<Pred(typename T<Args...>::value_type)>::type> select(T<Args...> &container,
                                                                                             Pred pred) {
        using value_type = typename T<Args...>::value_type;

        const auto &iter = _decl::projection_range<T<Args...>, Pred>{container, pred};
        return T<typename std::result_of<Pred(value_type)>::type>{iter.begin(), iter.end()};
    }
    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr T<typename std::result_of<Pred(typename T<Args...>::value_type)>::type>
    select(const T<Args...> &container, Pred pred) {
        using value_type = typename T<Args...>::value_type;
        const auto &iter = _decl::projection_range<const T<Args...>, Pred>{container, pred};
        return T<typename std::result_of<Pred(value_type)>::type>{iter.begin(), iter.end()};
    }
    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr T<typename std::result_of<Pred(typename T<Args...>::value_type)>::type::first_type,
                typename std::result_of<Pred(typename T<Args...>::value_type)>::type::second_type>
    select(T<Args...> &container, Pred pred) {
        using first_type = typename std::result_of<Pred(typename T<Args...>::value_type)>::type::first_type;
        using second_type = typename std::result_of<Pred(typename T<Args...>::value_type)>::type::second_type;
        const auto &iter = _decl::projection_range<T<Args...>, Pred>{container, pred};
        return T<first_type, second_type>{iter.begin(), iter.end()};
    }
    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr T<typename std::result_of<Pred(typename T<Args...>::value_type)>::type::first_type,
                typename std::result_of<Pred(typename T<Args...>::value_type)>::type::second_type>
    select(const T<Args...> &container, Pred pred) {
        using first_type = typename std::result_of<Pred(typename T<Args...>::value_type)>::type::first_type;
        using second_type = typename std::result_of<Pred(typename T<Args...>::value_type)>::type::second_type;
        const auto &iter = _decl::projection_range<const T<Args...>, Pred>{container, pred};
        return T<first_type, second_type>{iter.begin(), iter.end()};
    }
    template<template<typename, typename> class T, typename T1, typename T2, typename Alloc1, typename Alloc2,
             typename AllocResult>
    constexpr T<std::pair<T1, T2>, AllocResult> zip(T<T1, Alloc1> &first, T<T2, Alloc2> &second) {
        if (first.size() != second.size())
            throw std::runtime_error("Containers cannot have different size.");

        using size_type = typename T<T1, Alloc1>::size_type;
        size_type size = first.size();
        T<std::pair<T1, T2>, AllocResult> result(size);
        for (size_type i{0}; i < size; ++i) {
            result[i] = std::make_pair(first.at(i), second.at(i));
        }

        return result;
    }
    template<template<typename, typename> class T, typename T1, typename T2, typename Alloc1, typename Alloc2,
             typename AllocResult>
    constexpr T<std::pair<T1, T2>, AllocResult> zip(const T<T1, Alloc1> &first, const T<T2, Alloc2> &second) {
        if (first.size() != second.size())
            throw std::runtime_error("Containers cannot have different size.");

        using size_type = typename T<T1, Alloc1>::size_type;
        size_type size = first.size();
        T<std::pair<T1, T2>, AllocResult> result(size);
        for (size_type i{0}; i < size; ++i) {
            result[i] = std::make_pair(first.at(i), second.at(i));
        }

        return result;
    }
    template<template<typename...> class T, typename... Args>
    constexpr T<std::pair<typename T<Args...>::value_type, typename T<Args...>::value_type>>
    zip(const T<Args...> &first, const T<Args...> &second) {
        if (first.size() != second.size())
            throw std::runtime_error("Containers cannot have different size.");

        using value_type = typename T<Args...>::value_type;
        using size_type = typename T<Args...>::size_type;
        size_type size = first.size();
        T<std::pair<value_type, value_type>> result(size);
        for (size_type i{0}; i < size; ++i) {
            result[i] = std::make_pair(first.at(i), second.at(i));
        }

        return result;
    }
    // template<typename _T1, typename _T2, typename _Alloc1, typename _Alloc2, typename _AllocResult>
    // constexpr std::list<std::pair<_T1, _T2>, _AllocResult> zip(std::list<_T1, _Alloc1> &first,
    //                                                            std::list<_T2, _Alloc2> &second) {
    //     if (first.size() != second.size())
    //         throw std::runtime_error("Containers cannot have different size.");
    //
    //     std::list<std::pair<_T1, _T2>, _AllocResult> result{};
    //
    //     auto it_first = first.begin();
    //     auto it_second = second.begin();
    //
    //     while (it_first != first.end()) {
    //         result.emplace_back(std::make_pair(*it_first, *it_second));
    //
    //         ++it_first;
    //         ++it_second;
    //     }
    //
    //     return result;
    // }
    // template<typename _T1, typename _T2, typename _Alloc1, typename _Alloc2, typename _AllocResult>
    // constexpr std::list<std::pair<_T1, _T2>, _AllocResult> zip(const std::list<_T1, _Alloc1> &first,
    //                                                            const std::list<_T2, _Alloc2> &second) {
    //     if (first.size() != second.size())
    //         throw std::runtime_error("Containers cannot have different size.");
    //
    //     std::list<std::pair<_T1, _T2>, _AllocResult> result{};
    //
    //     auto it_first = first.begin();
    //     auto it_second = second.begin();
    //
    //     while (it_first != first.end()) {
    //         result.emplace_back(std::make_pair(*it_first, *it_second));
    //
    //         ++it_first;
    //         ++it_second;
    //     }
    //
    //     return result;
    // }
    template<template<typename, std::size_t> class T, typename value_type, std::size_t N>
    constexpr T<std::pair<value_type, value_type>, N> zip(T<value_type, N> &first, T<value_type, N> &second) {
        T<std::pair<value_type, value_type>, N> result{};
        for (std::size_t i{0}; i < N; ++i) {
            result[i] = std::make_pair(first[i], second[i]);
        }

        return result;
    }
    template<template<typename, std::size_t> class T, typename value_type, std::size_t N>
    constexpr T<std::pair<value_type, value_type>, N> zip(const T<value_type, N> &first,
                                                          const T<value_type, N> &second) {
        T<std::pair<value_type, value_type>, N> result{};
        for (std::size_t i{0}; i < N; ++i) {
            result[i] = std::make_pair(first[i], second[i]);
        }

        return result;
    }
    template<_std_container_ T, class Compare>
    constexpr typename T::value_type max(const T &container, const Compare &cmp) {
        using size_type = typename T::size_type;
        typename T::value_type result{std::numeric_limits<typename T::value_type>::min()};
        const size_type iter_to = container.size();
        for (size_type i{0}; i < iter_to; ++i) {
            if (cmp(result, container.at(i)))
                result = container.at(i);
        }

        return result;
    }
    template<_std_container_ T, typename Compare>
    constexpr typename T::value_type max(T &container, const Compare &cmp) {
        using size_type = typename T::size_type;
        typename T::value_type result{std::numeric_limits<typename T::value_type>::min()};
        const size_type iter_to = container.size();
        for (size_type i{0}; i < iter_to; ++i) {
            if (cmp(result, container.at(i)))
                result = container.at(i);
        }

        return result;
    }
    template<_std_container_ T, typename Compare>
    constexpr typename T::value_type min(const T &container, const Compare &cmp) {
        using size_type = typename T::size_type;
        typename T::value_type result{std::numeric_limits<typename T::value_type>::max()};
        const size_type iter_to = container.size();
        for (size_type i{0}; i < iter_to; ++i) {
            if (cmp(result, container.at(i)))
                result = container.at(i);
        }

        return result;
    }
    template<_std_container_ T, typename Compare>
    constexpr typename T::value_type min(T &container, const Compare &cmp) {
        using size_type = typename T::size_type;
        typename T::value_type result{std::numeric_limits<typename T::value_type>::max()};
        const size_type iter_to = container.size();
        for (size_type i{0}; i < iter_to; ++i) {
            if (cmp(result, container.at(i)))
                result = container.at(i);
        }

        return result;
    }

#endif

} // namespace ranged

#endif // FLUX_H
