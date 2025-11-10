//
// Created by mmatz on 9/3/25.
//

#ifndef RANGED_H
#define RANGED_H
#include <algorithm>
#include <cstddef>
#include <ranged.h>
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

        template<typename... Args>
        using void_t = void;

        template<typename T, typename = void>
        struct has_pair_types_impl : std::false_type {};

        template<typename T>
        struct has_pair_types_impl<T, void_t<typename T::first_type, typename T::second_type>> : std::true_type {};

        template<typename T>
#if __cplusplus >= 201402L
        struct has_pair_types : has_pair_types_impl<std::decay_t<T>> {};
#else
        struct has_pair_types : has_pair_types_impl<typename std::decay<T>::type> {};
#endif

        template<typename Iter, typename Pred>
        class filter_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = typename std::iterator_traits<Iter>::value_type;
            using difference_type = typename std::iterator_traits<Iter>::difference_type;
            using pointer = typename std::iterator_traits<Iter>::pointer;
            using reference = typename std::iterator_traits<Iter>::reference;
#if __cplusplus >= 201402L
            using base_iterator = std::decay_t<Iter>;
#else
            using base_iterator = typename std::remove_const<typename std::remove_reference<Iter>::type>::type;
#endif

            filter_iterator() = default;

            template<typename I1, typename I2>
            filter_iterator(I1 &&begin, I2 &&end, Pred pred) noexcept :
                current_(std::forward<I1>(begin)), end_(std::forward<I2>(end)), pred_(pred) {
                satisfy();
            }

            constexpr reference operator*() const noexcept { return *current_; }
            constexpr pointer operator->() const noexcept { return &(*current_); }

            filter_iterator &operator++() {
                ++current_;
                satisfy();
                return *this;
            }

            filter_iterator operator++(int) {
                filter_iterator tmp = *this;
                ++*this;
                return tmp;
            }

            constexpr friend bool operator==(const filter_iterator &lhs, const filter_iterator &rhs) {
                return lhs.current_ == rhs.current_;
            }

            constexpr friend bool operator!=(const filter_iterator &lhs, const filter_iterator &rhs) {
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
        class filter {
        public:
            using IteratorType = decltype(std::begin(std::declval<Range &>()));
            using iterator = filter_iterator<IteratorType, Pred>;
            using value_type = typename std::iterator_traits<iterator>::value_type;
            using difference_type = typename std::iterator_traits<iterator>::difference_type;
            using pointer = typename std::iterator_traits<iterator>::pointer;
            using reference = typename std::iterator_traits<iterator>::reference;

            filter(Range &range, Pred pred) noexcept :
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
        class transform_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
#if __cplusplus >= 201402L
            using value_type = std::result_of_t<Pred(typename std::iterator_traits<Iter>::value_type)>;
#else
            using value_type = typename std::result_of<Pred(typename std::iterator_traits<Iter>::value_type)>::type;
#endif
            using difference_type = typename std::iterator_traits<Iter>::difference_type;
            using pointer = value_type *;
            using reference = value_type &;
#if __cplusplus >= 201402L
            using ibase_iterator = std::decay_t<Iter>;
#else
            using ibase_iterator = typename std::decay<Iter>::type;
#endif

            transform_iterator() = default;

            template<typename I1, typename I2>
            transform_iterator(I1 &&begin, I2 &&end, Pred pred) noexcept :
                current_(std::forward<I1>(begin)), end_(std::forward<I2>(end)), pred_(pred) {}

            constexpr value_type operator*() const noexcept { return pred_(*current_); }
            constexpr pointer operator->() const noexcept { return &pred_(*current_); }

            transform_iterator &operator++() {
                ++current_;
                return *this;
            }

            transform_iterator operator++(int) {
                transform_iterator tmp = *this;
                ++*this;
                return tmp;
            }

            constexpr friend bool operator==(const transform_iterator &lhs, const transform_iterator &rhs) {
                return lhs.current_ == rhs.current_;
            }

            constexpr friend bool operator!=(const transform_iterator &lhs, const transform_iterator &rhs) {
                return lhs.current_ != rhs.current_;
            }

        private:
            ibase_iterator current_{};
            ibase_iterator end_{};
            Pred pred_;
        };
        template<typename Range, typename Pred>
        class transform {
        public:
            using IteratorType = decltype(std::begin(std::declval<Range &>()));
            using iterator = transform_iterator<IteratorType, Pred>;
            using value_type = typename std::iterator_traits<iterator>::value_type;
            using difference_type = typename std::iterator_traits<iterator>::difference_type;
            using pointer = typename std::iterator_traits<iterator>::pointer;
            using reference = typename std::iterator_traits<iterator>::reference;

            transform(Range &range, Pred pred) noexcept :
                begin_it(std::begin(range), std::end(range), pred), end_it(std::end(range), std::end(range), pred) {}

            iterator begin() { return begin_it; }
            iterator end() { return end_it; }
            iterator begin() const { return begin_it; }
            iterator end() const { return end_it; }

        private:
            iterator begin_it;
            iterator end_it;
        };

        template<typename I1, typename I2>
        class zip_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using first_type = typename std::iterator_traits<I1>::value_type;
            using second_type = typename std::iterator_traits<I2>::value_type;
            using first_type_ref = typename std::iterator_traits<I1>::reference;
            using second_type_ref = typename std::iterator_traits<I2>::reference;
            using value_type = std::tuple<first_type, second_type>;
            using difference_type = std::ptrdiff_t;
            using pointer = void;
            using reference = std::tuple<first_type_ref, second_type_ref>;
#if __cplusplus >= 201402L
            using base_iterator1 = std::decay_t<I1>;
            using base_iterator2 = std::decay_t<I2>;
#else
            using base_iterator1 = typename std::decay<I1>::type;
            using base_iterator2 = typename std::decay<I2>::type;
#endif

            zip_iterator() = default;

            zip_iterator(I1 &&begin1, I2 &&begin2, I1 &&end1, I2 &&end2) noexcept :
                current_1(std::forward<I1>(begin1)), current_2(std::forward<I2>(begin2)), end_1(std::forward<I1>(end1)), end_2(std::forward<I2>(end2)) {
            }

            constexpr reference operator*() const noexcept { return std::tie(*current_1, *current_2); }
            constexpr pointer operator->() const = delete;

            zip_iterator &operator++() {
                ++current_1;
                ++current_2;
                return *this;
            }

            zip_iterator operator++(int) {
                zip_iterator tmp = *this;
                ++*this;
                return tmp;
            }

            constexpr friend bool operator==(const zip_iterator &lhs, const zip_iterator &rhs) {
                return lhs.current_1 == rhs.current_1 && lhs.current_2 == rhs.current_2;
            }

            constexpr friend bool operator!=(const zip_iterator &lhs, const zip_iterator &rhs) {
                return lhs.current_1 != rhs.current_1 || lhs.current_2 != rhs.current_2;
            }

        private:
            base_iterator1 current_1{};
            base_iterator2 current_2{};
            base_iterator1 end_1{};
            base_iterator2 end_2{};
        };

        template<typename R1, typename R2>
        class zip {
        public:
            using IteratorType1 = decltype(std::begin(std::declval<R1 &>()));
            using IteratorType2 = decltype(std::begin(std::declval<R2 &>()));
            using iterator = zip_iterator<IteratorType1, IteratorType2>;
            using value_type = typename std::iterator_traits<iterator>::value_type;
            using difference_type = typename std::iterator_traits<iterator>::difference_type;
            using pointer = typename std::iterator_traits<iterator>::pointer;
            using reference = typename std::iterator_traits<iterator>::reference;

            zip(R1 &first_range, R2 &second_range) noexcept:
                begin_it(std::begin(first_range), std::begin(second_range), std::end(first_range), std::end(second_range)), end_it(std::end(first_range), std::end(second_range), std::end(first_range), std::end(second_range)) {}

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

    template<template<typename, typename...> class Tt, class Tf>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::to` instead")]]
#endif
    constexpr Tt<typename Tf::value_type> to(const Tf &container);
    template<template<typename, typename...> class Tt, class Tf>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::to` instead")]]
#endif
    constexpr Tt<typename Tf::value_type> to(Tf &container);
    template<template<typename, typename...> class Tt, class Tf>
    constexpr Tt<typename std::decay<typename Tf::value_type::first_type>::type, typename Tf::value_type::second_type>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::to` instead")]]
#endif
    to(const Tf &container);
    template<template<typename, typename...> class Tt, class Tf>
    constexpr Tt<typename std::decay<typename Tf::value_type::first_type>::type, typename Tf::value_type::second_type>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::to` instead")]]
#endif
    to(Tf &container);
    template<std::size_t N, _std_container_ T>
    constexpr std::array<typename T::value_type, N> to_array(T &container);
    template<std::size_t N, _std_container_ T>
    constexpr std::array<typename T::value_type, N> to_array(const T &container);
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
    constexpr _decl::filter<T, Pred> filter(T &container, Pred func);
    template<_std_container_ T, typename Pred>
    constexpr _decl::filter<const T, Pred> filter(const T &container, Pred func);
    template<typename T, size_t N, typename Pred, class AllocT = std::allocator<T>>
    constexpr std::vector<T> filter(const std::array<T, N> &array, Pred pred);

    template<template<typename...> class T, typename... Args, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::transform` instead")]]
#endif
    constexpr _decl::transform<T<Args...>, Pred> transform(T<Args...> &container, const Pred &pred);
    template<template<typename...> class T, typename... Args, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::transform` instead")]]
#endif
    constexpr _decl::transform<const T<Args...>, Pred> transform(const T<Args...> &container, const Pred &pred);

    template<_std_container_ T, _std_container_ U>
    constexpr _decl::zip<T, U> zip(T &first, U &second);
    template<_std_container_ T, _std_container_ U>
    constexpr _decl::zip<const T, const U> zip(const T &first, const U &second);
    template<template<typename, std::size_t> class Ta, typename T1, typename T2, std::size_t N>
    constexpr Ta<std::tuple<T1, T2>, N> zip(Ta<T1, N> &first, Ta<T2, N> &second);
    template<template<typename, std::size_t> class Ta, typename T1, typename T2, std::size_t N>
    constexpr Ta<std::tuple<T1, T2>, N> zip(const Ta<T1, N> &first, const Ta<T2, N> &second);


#ifdef RANGED_IMPLEMENTATION


    template<_std_container_ T, typename Pred>
    constexpr bool any(const T &container, const Pred &func) {
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

    template<_std_container_ T, typename Pred>
    constexpr bool all(const T &container, const Pred &func) {
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
    template<_std_container_ T, typename Pred>
    constexpr auto first_or_default(const T &container, const Pred &func, const typename T::value_type &default_value)
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
    template<template<typename, typename...> class Tt, class Tf>
    constexpr Tt<typename Tf::value_type> to(const Tf &container) {
        return Tt<typename Tf::value_type>(std::begin(container), std::end(container));
    }
    template<template<typename, typename...> class Tt, class Tf>
    constexpr Tt<typename Tf::value_type> to(Tf &container) {
        return Tt<typename Tf::value_type>(std::begin(container), std::end(container));
    }
    template<template<typename, typename...> class Tt, class Tf>
    constexpr Tt<typename std::decay<typename Tf::value_type::first_type>::type, typename Tf::value_type::second_type>
    to(const Tf &container) {
        return Tt<typename std::decay<typename Tf::value_type::first_type>::type, typename Tf::value_type::second_type>(
                std::begin(container), std::end(container));
    }
    template<template<typename, typename...> class Tt, class Tf>
    constexpr Tt<typename std::decay<typename Tf::value_type::first_type>::type, typename Tf::value_type::second_type>
    to(Tf &container) {
        return Tt<typename std::decay<typename Tf::value_type::first_type>::type, typename Tf::value_type::second_type>(
                std::begin(container), std::end(container));
    }
    template<std::size_t N, _std_container_ T>
    constexpr std::array<typename T::value_type, N> to_array(T &container) {
        std::array<typename T::value_type, N> result{};
        const size_t iter_to = std::min(N, container.size());
        for (size_t i{0}; i < iter_to; ++i) {
            result[i] = container.at(i);
        }

        return result;
    }
    template<std::size_t N, _std_container_ T>
    constexpr std::array<typename T::value_type, N> to_array(const T &container) {
        std::array<typename T::value_type, N> result{};
        const size_t iter_to = std::min(N, container.size());
        for (size_t i{0}; i < iter_to; ++i) {
            result[i] = container.at(i);
        }

        return result;
    }
    template<_std_container_ T, typename Pred>
    constexpr size_t count_if(const T &container, const Pred &func) {
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
    template<_std_container_ T, typename Pred>
    constexpr _decl::filter<T, Pred> filter(T &container, Pred func) {
        return _decl::filter<T, Pred>{container, func};
    }
    template<_std_container_ T, typename Pred>
    constexpr _decl::filter<const T, Pred> filter(const T &container, Pred func) {
        return _decl::filter<const T, Pred>{container, func};
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
    constexpr _decl::transform<T<Args...>, Pred> transform(T<Args...> &container, const Pred &pred) {
        return _decl::transform<T<Args...>, Pred>{container, pred};
    }
    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr _decl::transform<const T<Args...>, Pred> transform(const T<Args...> &container, const Pred &pred) {
        return _decl::transform<const T<Args...>, Pred>{container, pred};
    }
    template<_std_container_ T, _std_container_ U>
    constexpr _decl::zip<T, U> zip(T &first, U &second) {
        if (first.size() != second.size())
            throw std::runtime_error("Containers cannot have different size.");
        return _decl::zip<T, U>{first, second};
    }
    template<_std_container_ T, _std_container_ U>
    constexpr _decl::zip<const T, const U> zip(const T &first, const U &second) {
        if (first.size() != second.size()) throw std::runtime_error("Containers cannot have different size.");
        return _decl::zip<const T, const U>{first, second};
    }
    template<template<typename, std::size_t> class Ta, typename T1, typename T2, std::size_t N>
    constexpr Ta<std::tuple<T1, T2>, N> zip(Ta<T1, N> &first, Ta<T2, N> &second) {
        Ta<std::tuple<T1, T2>, N> result{};
        for (std::size_t i{0}; i < N; ++i) {
            result[i] = std::make_tuple(first[i], second[i]);
        }

        return result;
    }
    template<template<typename, std::size_t> class Ta, typename T1, typename T2, std::size_t N>
    constexpr Ta<std::tuple<T1, T2>, N> zip(const Ta<T1, N> &first, const Ta<T2, N> &second) {
        Ta<std::tuple<T1, T2>, N> result{};
        for (std::size_t i{0}; i < N; ++i) {
            result[i] = std::make_pair(first[i], second[i]);
        }

        return result;
    }
    template<template<typename, std::size_t> class Ta, typename T1, typename T2, std::size_t N>
    constexpr Ta<std::pair<T1, T2>, N> zip(const Ta<T1, N> &first, const Ta<T2, N> &second) {
        Ta<std::pair<T1, T2>, N> result{};
        for (std::size_t i{0}; i < N; ++i) {
            result[i] = std::make_pair(first[i], second[i]);
        }

        return result;
    }
    template<_std_container_ T, class Compare>
    constexpr typename T::value_type max(const T &container, const Compare &cmp) {
        if (container.empty())
            return std::numeric_limits<typename T::value_type>::min();
        auto it = std::begin(container);
        typename T::value_type result = *it;
        const auto end = std::end(container);

        for (++it; it != end; ++it) {
            if (cmp(result, *it))
                result = *it;
        }

        return result;
    }
    template<_std_container_ T, typename Compare>
    constexpr typename T::value_type max(T &container, const Compare &cmp) {
        if (container.empty())
            return std::numeric_limits<typename T::value_type>::min();
        auto it = std::begin(container);
        typename T::value_type result = *it;
        const auto end = std::end(container);

        for (++it; it != end; ++it) {
            if (cmp(result, *it))
                result = *it;
        }

        return result;
    }
    template<_std_container_ T, typename Compare>
    constexpr typename T::value_type min(const T &container, const Compare &cmp) {
        if (container.empty())
            return std::numeric_limits<typename T::value_type>::max();
        auto it = std::begin(container);
        typename T::value_type result = *it;
        const auto end = std::end(container);

        for (++it; it != end; ++it) {
            if (cmp(result, *it))
                result = *it;
        }

        return result;
    }
    template<_std_container_ T, typename Compare>
    constexpr typename T::value_type min(T &container, const Compare &cmp) {
        if (container.empty())
            return std::numeric_limits<typename T::value_type>::max();
        auto it = std::begin(container);
        typename T::value_type result = *it;
        const auto end = std::end(container);

        for (++it; it != end; ++it) {
            if (cmp(result, *it))
                result = *it;
        }

        return result;
    }

#endif

} // namespace ranged

#endif // FLUX_H
