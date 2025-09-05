//
// Created by mmatz on 9/3/25.
//

#ifndef RANGED_H
#define RANGED_H
#include <algorithm>
#include <cstddef>
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

        template<class T>
        struct less;

        template<class T = void>
        struct more {
            constexpr bool operator()(const T &lhs, const T &rhs) const { return lhs > rhs; };
        };
        template<class T = void>
        struct less {
            constexpr bool operator()(const T &lhs, const T &rhs) const { return lhs < rhs; };
        };

        template<typename Iter, typename Pred>
        class conditional_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = typename std::iterator_traits<Iter>::value_type;
            using difference_type = typename std::iterator_traits<Iter>::difference_type;
            using pointer = typename std::iterator_traits<Iter>::pointer;
            using reference = typename std::iterator_traits<Iter>::reference;
            using base_iterator = typename std::remove_const<typename std::remove_reference<Iter>::type>::type;

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
            using value_type = typename std::result_of<Pred(typename std::iterator_traits<Iter>::value_type)>::type;
            using difference_type = typename std::iterator_traits<Iter>::difference_type;
            using pointer = value_type *;
            using reference = value_type &;
            using ibase_iterator = typename std::remove_const<typename std::remove_reference<Iter>::type>::type;

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

    template<_std_container_ T, typename Cond>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::any_of` instead")]]
#endif
    constexpr bool any(const T &container, const Cond &func);
    template<_std_container_ T, typename Cond>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::all_of` instead")]]
#endif
    constexpr bool all(const T &container, const Cond &func);
    template<_std_container_ T>
    constexpr bool contains(const T &container, const typename T::value_type &value);
    template<_std_container_ T, typename Func>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::for_each` instead")]]
#endif
    constexpr void for_each(const T &container, const Func &func);
    template<_std_container_ T, typename Cond>
    constexpr auto first_or_default(const T &container, const Cond &func,
                                    const typename T::value_type &default_value = typename T::value_type()) ->
            typename T::value_type;

    template<_std_container_ T, typename AllocT = std::allocator<T>>
    constexpr auto to_vector(const T &container) -> std::vector<typename T::value_type, AllocT>;
    template<size_t N, _std_container_ T>
    constexpr auto to_array(const T &container) -> std::array<typename T::value_type, N>;
    template<_std_container_ T, typename Cond>
    constexpr size_t count_if(const T &container, const Cond &func);
#if __cplusplus >= 201402L
    template<_std_container_ T, typename Compare = std::less<typename T::value_type>>
#else
    template<_std_container_ T, typename Compare = _decl::less<typename T::value_type>>
#endif
    constexpr typename T::value_type max(const T &container, const Compare &cmp = {});
    template<_std_container_ T, typename Compare = _decl::more<typename T::value_type>>
    constexpr typename T::value_type min(const T &container, const Compare &cmp = {});

    // Iterator based
    template<_std_container_ T, typename Cond>
    constexpr T filter(T &container, Cond func);
    template<_std_container_ T, typename Cond>
    constexpr T filter(const T &container, Cond func);
    template<typename T, size_t N, typename Pred, typename AllocT = std::allocator<T>>
    constexpr std::vector<T> filter(const std::array<T, N> &array, Pred pred);

    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr T<typename std::result_of<Pred(typename T<Args...>::value_type)>::type> select(T<Args...> &container,
                                                                                             Pred pred);
    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr T<typename std::result_of<Pred(typename T<Args...>::value_type)>::type>
    select(const T<Args...> &container, Pred pred);

    // TODO: Implement `select` function for `std::map` & `std::unordered_map`
    // template<class Key, class T, class Compare = std::less<Key>, class Alloc = std::allocator<std::pair<const Key,
    // T>>,
    //          typename Pred>
    // constexpr std::map<
    //         typename std::result_of<Pred(typename std::map<Key, T, Compare, Alloc>::value_type)>::first_type,
    //         typename std::result_of<Pred(typename std::map<Key, T, Compare, Alloc>::value_type)>::second_type,
    //         Compare, Alloc>
    // select(std::map<Key, T, Compare, Alloc> &container, Pred pred);
    // template<class Key, class T, class Compare = std::less<Key>, class Alloc = std::allocator<std::pair<const Key,
    // T>>,
    //          typename Pred>
    // constexpr std::map<
    //         typename std::result_of<Pred(typename std::map<Key, T, Compare, Alloc>::value_type)>::first_type,
    //         typename std::result_of<Pred(typename std::map<Key, T, Compare, Alloc>::value_type)>::second_type,
    //         Compare, Alloc>
    // select(const std::map<Key, T, Compare, Alloc> &container, Pred pred);


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
    template<_std_container_ T>
    constexpr bool contains(const T &container, const typename T::value_type &value) {
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
    template<_std_container_ T, typename Cond>
    constexpr auto first_or_default(const T &container, const Cond &func, const typename T::value_type &default_value)
            -> typename T::value_type {
        for (const auto &item: container) {
            if (func(item))
                return item;
        }

        return default_value;
    }
    template<_std_container_ T, typename AllocT>
    constexpr auto to_vector(const T &container) -> std::vector<typename T::value_type, AllocT> {
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
    template<_std_container_ T, typename Cond>
    constexpr size_t count_if(const T &container, const Cond &func) {
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
    template<typename T, size_t N, typename Pred, typename AllocT>
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
    // template<class Key, class T, class Compare, class Alloc, typename Pred>
    // constexpr std::map<
    //         typename std::result_of<Pred(typename std::map<Key, T, Compare, Alloc>::value_type)>::first_type,
    //         typename std::result_of<Pred(typename std::map<Key, T, Compare, Alloc>::value_type)>::second_type,
    //         Compare, Alloc>
    // select(const std::map<Key, T, Compare, Alloc> &container, Pred pred) {
    //     using predicate_type = Pred(typename std::map<Key, T, Compare, Alloc>::value_type);
    //     using key_type = typename std::result_of<predicate_type>::first_type;
    //     using item_type = typename std::result_of<predicate_type>::second_type;
    //     std::map<key_type, item_type, Compare, Alloc> result;
    //
    //     for (const auto &item: container) {
    //         result.insert(pred(item));
    //     }
    //
    //     return result;
    // }
    // template<class Key, class T, class Compare, class Alloc, typename Pred>
    // constexpr std::map<
    //         typename std::result_of<Pred(typename std::map<Key, T, Compare, Alloc>::value_type)>::first_type,
    //         typename std::result_of<Pred(typename std::map<Key, T, Compare, Alloc>::value_type)>::second_type,
    //         Compare, Alloc>
    // select(std::map<Key, T, Compare, Alloc> &container, Pred pred) {
    //     using predicate_type = Pred(typename std::map<Key, T, Compare, Alloc>::value_type);
    //     using key_type = typename std::result_of<predicate_type>::first_type;
    //     using item_type = typename std::result_of<predicate_type>::second_type;
    //     std::map<key_type, item_type, Compare, Alloc> result;
    //
    //     for (const auto &item: container) {
    //         result.insert(pred(item));
    //     }
    //
    //     return result;
    // }

    template<_std_container_ T, class Compare>
    constexpr typename T::value_type max(const T &container, const Compare &cmp) {
        typename T::value_type result{std::numeric_limits<typename T::value_type>::min()};
        const size_t iter_to = container.size();
        for (size_t i{0}; i < iter_to; ++i) {
            if (cmp(result, container.at(i)))
                result = container.at(i);
        }

        return result;
    }
    template<_std_container_ T, typename Compare>
    constexpr typename T::value_type min(const T &container, const Compare &cmp) {
        typename T::value_type result{std::numeric_limits<typename T::value_type>::max()};
        const size_t iter_to = container.size();
        for (size_t i{0}; i < iter_to; ++i) {
            if (cmp(result, container.at(i)))
                result = container.at(i);
        }

        return result;
    }

#endif

} // namespace ranged

#endif // FLUX_H
