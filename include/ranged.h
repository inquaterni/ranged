//
// Created by mmatz on 9/3/25.
//

#ifndef RANGED_H
#define RANGED_H
#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>
#include <functional>

#ifndef RANGED_NO_DEPRECATION_WARNINGS
#define RANGED_NO_DEPRECATION_WARNINGS 0
#endif

namespace ranged {

#if __cplusplus >= 202002L
    template<typename T>
    concept _std_container_ = requires(T a) {
        typename T::iterator;
        typename T::value_type;
        typename T::difference_type;
        typename T::pointer;
        typename T::reference;
    };
#else
#define std_container class
#endif

    // c++14's possible `std::exchange` implementation for c++11, see: https://en.cppreference.com/w/cpp/utility/exchange.html
    // TODO: replace all occurrences of this function for `std::exchange` for c++14 and newer
    template<class T, class U = T>
    constexpr T exchange(T& obj, U&& new_value) noexcept (
        std::is_nothrow_move_constructible<U>::value &&
        std::is_nothrow_assignable<T&, U>::value
    )
    {
        T tmp = std::move(obj);
        obj = std::forward<U>(new_value);
        return tmp;
    }

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

    template<typename T, typename = void>
    struct has_reserve : std::false_type {};
    template<typename T>
    struct has_reserve<T, void_t<decltype(std::declval<T>().reserve(1))>> : std::true_type {};
    template<typename T, typename = void>
    struct has_index_read : std::false_type {};
    template<typename T>
    struct has_index_read<T, void_t<decltype(std::declval<T>().operator[](0))>> : std::true_type {};
    template<typename T, typename = void>
    struct has_emplace_back : std::false_type {};
    template<typename T>
    struct has_emplace_back<T, void_t<decltype(std::declval<T>().emplace_back(std::declval<typename T::value_type>()))>> : std::true_type {};
    template<typename T, typename = void>
    struct is_empty : std::false_type {};
    template<typename T>
    struct is_empty<T, void_t<decltype(std::declval<T>().empty(std::declval<typename T::value_type&&>()))>> : std::true_type {};
    template<typename T, typename = void>
    struct sized : std::false_type {};
    template<typename T>
    struct sized<T, void_t<decltype(std::declval<T>().size())>> : std::true_type {};

    template<typename T>
#if __cplusplus >= 201304L
    struct has_pair_types : has_pair_types_impl<std::decay_t<T>> {};
#else
    struct has_pair_types : has_pair_types_impl<typename std::decay<T>::type> {};
#endif

    template<typename T>
    struct function_traits : function_traits<decltype(&std::decay<T>::type::operator())> {};

    template<typename ReturnT, typename... Args>
    struct function_traits<ReturnT(Args...)> {
        using signature = ReturnT(Args...);
        using return_type = ReturnT;
        // using arguments = std::tuple<Args...>;
    };
    template<typename ReturnT, typename... Args>
    struct function_traits<ReturnT(*)(Args...)> : function_traits<ReturnT(Args...)> {};
    template<typename ClassT, typename ReturnT, typename... Args>
    struct function_traits<ReturnT(ClassT::*)(Args...)> : function_traits<ReturnT(Args...)> {};
    template<typename ClassT, typename ReturnT, typename... Args>
    struct function_traits<ReturnT(ClassT::*)(Args...) const> : function_traits<ReturnT(Args...)> {};

    template<typename T>
    using function_traits_s = typename function_traits<T>::signature;
    template<typename T>
    using function_traits_rt = typename function_traits<T>::return_type;

    template<typename Pred>
    using is_bool_predicate = std::is_same<function_traits_rt<Pred>, bool>;

    namespace views {
        template<typename R>
        class owning_view {
        public:
            static_assert(std::is_object<R>::value, "Template parameter `R` must be an object type");
            static_assert(std::is_move_constructible<R>::value, "Template parameter `R` must be move constructible");
            static_assert(std::is_default_constructible<R>::value, "Template parameter `R` must be default constructible");

            using iterator = typename std::decay<R>::type::iterator;
            using const_iterator = typename std::decay<R>::type::const_iterator;
            using size_type = typename std::decay<R>::type::size_type;

            owning_view() noexcept : _r() {};
            ~owning_view() = default;
            owning_view(owning_view&& other) noexcept : _r(exchange(other._r, R {})) {};
            owning_view& operator=(owning_view&& other) = default;

            owning_view(const owning_view&) = delete;
            owning_view& operator=(const owning_view&) = delete;

            explicit owning_view(R&& t) noexcept: _r {exchange(t, R {})} {}

            R &base() & noexcept { return _r; }
            constexpr const R &base() const & noexcept { return this->_r; }
            constexpr R &&base() && noexcept { return std::move(_r); }

            iterator begin() noexcept { return _r.begin(); }
            iterator end() noexcept { return _r.end(); }

            const_iterator begin() const noexcept { return _r.begin(); }
            const_iterator end() const noexcept { return _r.end(); }

            constexpr bool empty() const noexcept { return is_empty<R>::value; }
            bool empty() noexcept { return is_empty<R>::value; }

            size_type size() noexcept (
                sized<R>::value
            ) {
                return std::distance(std::move(this->_r.begin()), std::move(this->_r.end()));
            }
            constexpr size_type size() const noexcept (
                sized<R>::value
            ) {
                return std::distance(std::move(this->_r.begin()), std::move(this->_r.end()));
            }

        protected:
            R _r;
        };
        template<typename R>
        class ref_view {
        public:
            static_assert(std::is_object<R>::value, "Template parameter `R` must be an object type");

            using iterator = typename std::decay<R>::type::iterator;
            using const_iterator = typename std::decay<R>::type::const_iterator;
            using size_type = typename std::decay<R>::type::size_type;

            ref_view() noexcept : _r(nullptr) {}

            template<typename T>
            constexpr explicit ref_view(T& t) noexcept(
                std::is_convertible<T, R&>::value
                ) : _r(std::addressof(static_cast<R&>(std::forward<T>(t)))) {}

            constexpr explicit ref_view(R* t) noexcept : _r(t) {}


            ref_view(const ref_view&) = default;
            ref_view& operator=(const ref_view&) = default;
            ref_view(ref_view&&) = default;
            ref_view& operator=(ref_view&&) = default;
            ~ref_view() = default;

            constexpr R& base() const noexcept {
                assert(_r != nullptr);
                return *_r;
            }

            iterator begin() const noexcept {
                assert(_r != nullptr);
                return _r->begin();
            }
            iterator end() const noexcept {
                assert(_r != nullptr);
                return _r->end();
            }

            constexpr bool empty() const noexcept {
                assert(_r != nullptr);
                return _r->empty();
            }

            size_type size() const noexcept {
                assert(_r != nullptr);
                return _r->size();
            }

        protected:
            R* _r;
        };

        template<typename Iter, typename Pred>
        class filter_iterator {
        public:
#if __cplusplus >= 201304L
            using base_iterator = std::decay_t<Iter>;
#else
            using base_iterator = typename std::decay<Iter>::type;
#endif
            using iterator_category = std::forward_iterator_tag;
            using value_type = typename std::iterator_traits<base_iterator>::value_type;
            using difference_type = typename std::iterator_traits<base_iterator>::difference_type;
            using pointer = typename std::iterator_traits<base_iterator>::pointer;
            using reference = typename std::iterator_traits<base_iterator>::reference;
            using function_type = std::function<function_traits_s<Pred>>;

            filter_iterator() = default;

            filter_iterator(const filter_iterator &other) noexcept(
                std::is_nothrow_copy_constructible<base_iterator>::value
                ): current_(other.current_), end_(other.end_), pred_(other.pred_) {}
            filter_iterator& operator=(const filter_iterator &other) noexcept(
                std::is_nothrow_copy_assignable<base_iterator>::value) {
                if (&other != this) {
                    current_ = other.current_;
                    end_ = other.end_;
                    pred_ = other.pred_;
                }

                return *this;
            };

            filter_iterator(filter_iterator &&rhs) noexcept(
                std::is_nothrow_move_constructible<base_iterator>::value &&
                std::is_default_constructible<base_iterator>::value
                ):
#if __cplusplus >= 201304L
            current_(std::exchange(rhs.current_, base_iterator {})), end_(std::exchange(rhs.end_, base_iterator {})), pred_(std::exchange(rhs.pred_, function_type {})) {}
#else
            current_(exchange(rhs.current_, base_iterator {})), end_(exchange(rhs.end_, base_iterator {})), pred_(exchange(rhs.pred_, function_type {})) {}
#endif

            filter_iterator &operator=(filter_iterator &&rhs) noexcept(
                std::is_nothrow_move_assignable<base_iterator>::value &&
                std::is_default_constructible<base_iterator>::value
                )
            {
                if (&rhs != this) {
#if __cplusplus >= 201304L
                    current_ = std::exchange(rhs.current_, base_iterator {});
                    end_ = std::exchange(rhs.end_, base_iterator {});
                    pred_ = std::exchange(rhs.pred_, function_type {});
#else
                    current_ = exchange(rhs.current_, base_iterator {});
                    end_ = exchange(rhs.end_, base_iterator {});
                    pred_ = exchange(rhs.pred_, function_type {});
#endif
                }

                return *this;
            }

            filter_iterator(base_iterator begin, base_iterator end, const function_type &pred) noexcept :
            current_(begin), end_(end), pred_(pred) {
                satisfy();
            }

            constexpr reference operator*() const noexcept { return *current_; }
            constexpr pointer operator->() const = delete;

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

            base_iterator current_;
            base_iterator end_;
            function_type pred_;
        };

        template<typename Range, typename Pred>
        class filter_view : public owning_view<Range> {
        public:
            using range_iterator_type = typename std::decay<Range>::type::iterator;
            using range_const_iterator_type = typename std::decay<Range>::type::const_iterator;
            using iterator = filter_iterator<range_iterator_type, Pred>;
            using const_iterator = filter_iterator<range_const_iterator_type, Pred>;
            using value_type = typename std::iterator_traits<iterator>::value_type;
            using difference_type = typename std::iterator_traits<iterator>::difference_type;
            using pointer = typename std::iterator_traits<iterator>::pointer;
            using reference = typename std::iterator_traits<iterator>::reference;
            using function_type = std::function<function_traits_s<Pred>>;

            filter_view() noexcept: owning_view<Range>(), _pred() {};

            filter_view(Range&& range, const Pred &pred) noexcept : owning_view<Range>(std::forward<Range>(range)), _pred(pred) {}
            filter_view(Range& range, const Pred &pred) noexcept : owning_view<Range>(std::move(range)), _pred(pred) {}

            filter_view(filter_view &&other) noexcept : owning_view<Range>(exchange(other._r, Range {})), _pred(exchange(other._pred, function_type {})) {}
            filter_view &operator=(filter_view &&other) noexcept {
                if (&other != this) {
                    this->_r = exchange(other._r, Range {});
                    this->_pred = exchange(other._pred, function_type {});
                }

                return *this;
            }

            filter_view(filter_view &) = delete;
            filter_view &operator=(filter_view &) = delete;

            iterator begin() noexcept { return iterator{this->_r.begin(), this->_r.end(), _pred}; }
            iterator end() noexcept { return iterator{this->_r.end(), this->_r.end(), _pred}; }

            constexpr const_iterator begin() const noexcept { return const_iterator{this->_r.begin(), this->_r.end(), _pred}; }
            constexpr const_iterator end() const noexcept { return const_iterator{this->_r.end(), this->_r.end(), _pred}; }

        private:
            function_type _pred;
        };
        template<typename Range, typename Pred>
        class filter_ref_view : public ref_view<Range> {
            public:
            using range_iterator_type = typename std::decay<Range>::type::iterator;
            using range_const_iterator_type = typename std::decay<Range>::type::const_iterator;
            using iterator = filter_iterator<range_iterator_type, Pred>;
            using const_iterator = filter_iterator<range_const_iterator_type, Pred>;
            using value_type = typename std::iterator_traits<iterator>::value_type;
            using difference_type = typename std::iterator_traits<iterator>::difference_type;
            using pointer = typename std::iterator_traits<iterator>::pointer;
            using reference = typename std::iterator_traits<iterator>::reference;
            using function_type = std::function<function_traits_s<Pred>>;

            filter_ref_view() noexcept: ref_view<Range>(), _pred() {}
            filter_ref_view(Range&& range, const Pred &pred) noexcept : ref_view<Range>(std::forward<Range>(range)), _pred(pred) {}
            filter_ref_view(Range& range, const Pred &pred) noexcept : ref_view<Range>(std::move(range)), _pred(pred) {}

            filter_ref_view(filter_ref_view &&other) noexcept : ref_view<Range>(exchange(other._r, nullptr)), _pred(exchange(other._pred, function_type {})) {}
            filter_ref_view &operator=(filter_ref_view &&other) noexcept {
                if (&other != this) {
                    this->_r = exchange(other._r, nullptr);
                    this->_pred = exchange(other._pred, function_type {});
                }

                return *this;
            }

            filter_ref_view(filter_ref_view &other) noexcept : ref_view<Range>(other._r), _pred(other._pred) {};
            filter_ref_view &operator=(filter_ref_view &other) noexcept {
                if (&other != this) {
                    this->_r = other._r;
                    this->_pred = other._pred;
                }

                return *this;
            };

            iterator begin() noexcept { return iterator{this->_r->begin(), this->_r->end(), _pred}; }
            iterator end() noexcept { return iterator{this->_r->end(), this->_r->end(), _pred}; }

            constexpr const_iterator begin() const noexcept { return const_iterator{this->_r->begin(), this->_r->end(), _pred}; }
            constexpr const_iterator end() const noexcept { return const_iterator{this->_r->end(), this->_r->end(), _pred}; }


        private:
            function_type _pred;
        };


        template<typename Iter, typename Pred>
        class transform_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
#if __cplusplus >= 201304L
            using value_type = std::result_of_t<Pred(typename std::iterator_traits<Iter>::value_type)>;
#else
            using value_type = typename std::result_of<Pred(typename std::iterator_traits<Iter>::value_type)>::type;
#endif
            using difference_type = typename std::iterator_traits<Iter>::difference_type;
            using pointer = value_type *;
            using reference = value_type;
#if __cplusplus >= 201304L
            using base_iterator = std::decay_t<Iter>;
#else
            using base_iterator = typename std::decay<Iter>::type;
#endif
            using function_type = std::function<function_traits_s<Pred>>;

            transform_iterator() = default;

            transform_iterator(const transform_iterator &other) noexcept (
                std::is_nothrow_copy_constructible<base_iterator>::value
                ) : current_(other.current_), end_(other.end_), pred_(other.pred_) {};
            transform_iterator &operator=(const transform_iterator &other) noexcept (
                std::is_nothrow_copy_assignable<base_iterator>::value
                ) {
                if (&other != this) {
                    current_ = other.current_;
                    end_ = other.end_;
                    pred_ = other.pred_;
                }

                return *this;
            }

            transform_iterator(transform_iterator &&other) noexcept (
                std::is_nothrow_move_constructible<base_iterator>::value &&
                std::is_default_constructible<base_iterator>::value
                ) :
#if __cplusplus >= 201304L
            current_(std::exchange(other.current_, base_iterator {})), end_(std::exchange(other.end_, base_iterator {})), pred_(std::exchange(other.pred_, function_type {})) {};
#else
            current_(exchange(other.current_, base_iterator {})), end_(exchange(other.end_, base_iterator {})), pred_(exchange(other.pred_, function_type {})) {};
#endif
            transform_iterator &operator=(transform_iterator &&other) noexcept (
                std::is_nothrow_move_assignable<base_iterator>::value &&
                std::is_default_constructible<base_iterator>::value
                ) {
                if (&other != this) {
#if __cplusplus >= 201304L
                    current_ = std::exchange(other.current_, base_iterator {});
                    end_ = std::exchange(other.end_, base_iterator {});
                    pred_ = std::exchange(other.pred_, function_type {});
#else

                    current_ = exchange(other.current_, base_iterator {});
                    end_ = exchange(other.end_, base_iterator {});
                    pred_ = exchange(other.pred_, function_type {});
#endif
                }

                return *this;
            }

            transform_iterator(Iter &&begin, Iter &&end, const Pred &pred) noexcept (
                std::is_nothrow_constructible<Iter>::value
                ) :
#if __cplusplus >= 201304L
                current_(std::exchange(begin, Iter {})), end_(std::exchange(end, Iter {})), pred_(pred) {}
#else
                current_(exchange(begin, Iter {})), end_(exchange(end, Iter {})), pred_(pred) {}
#endif

            constexpr value_type operator*() const noexcept { return pred_(*current_); }
            constexpr pointer operator->() const = delete;

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
            base_iterator current_;
            base_iterator end_;
            function_type pred_;
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

            transform() = default;

            transform(const transform&) = delete;
            transform &operator=(const transform&) = delete;

            transform(transform &&rhs) noexcept : begin_it(rhs.begin_it), end_it(rhs.end_it) {};
            transform &operator=(transform &&rhs) noexcept {
                if (&rhs != this) {
                    begin_it = rhs.begin_it;
                    end_it = rhs.end_it;
                }

                return *this;
            };

            transform(Range &range, const Pred &pred) noexcept:
                begin_it(std::begin(range), std::end(range), pred), end_it(std::end(range), std::end(range), pred) {}
            transform(Range &&, const Pred &) = delete;

            iterator& begin() { return begin_it; }
            iterator& end() { return end_it; }
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
#if __cplusplus >= 201304L
            using base_iterator1 = std::decay_t<I1>;
            using base_iterator2 = std::decay_t<I2>;
#else
            using base_iterator1 = typename std::decay<I1>::type;
            using base_iterator2 = typename std::decay<I2>::type;
#endif

            zip_iterator() = default;

            zip_iterator(I1 &&begin1, I2 &&begin2, I1 &&end1, I2 &&end2) noexcept (
                std::is_nothrow_move_constructible<base_iterator1>::value && std::is_nothrow_move_constructible<base_iterator2>::value &&
                std::is_default_constructible<base_iterator1>::value && std::is_default_constructible<base_iterator2>::value
                ):
#if __cplusplus >= 201304L
                current_1(std::exchange(begin1, base_iterator1 {})), current_2(std::exchange(begin2, base_iterator2 {})), end_1(std::exchange(end1, base_iterator1 {})), end_2(std::exchange(end2, base_iterator2 {})) {
#else
                current_1(exchange(begin1, base_iterator1 {})), current_2(exchange(begin2, base_iterator2 {})), end_1(exchange(end1, base_iterator1 {})), end_2(exchange(end2, base_iterator2 {})) {
#endif
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
                return lhs.current_1 == rhs.current_1 || lhs.current_2 == rhs.current_2;
            }

            constexpr friend bool operator!=(const zip_iterator &lhs, const zip_iterator &rhs) {
                return lhs.current_1 != rhs.current_1 && lhs.current_2 != rhs.current_2;
            }

        private:
            base_iterator1 current_1;
            base_iterator2 current_2;
            base_iterator1 end_1;
            base_iterator2 end_2;
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

            zip() = default;

            zip(R1 &first_range, R2 &second_range) noexcept:
                begin_it(std::begin(first_range), std::begin(second_range), std::end(first_range), std::end(second_range)), end_it(std::end(first_range), std::end(second_range), std::end(first_range), std::end(second_range)) {}

            zip(R1 &&, R2 &&) = delete;


            iterator& begin() { return begin_it; }
            iterator& end() { return end_it; }
            iterator begin() const { return begin_it; }
            iterator end() const { return end_it; }

        private:
            iterator begin_it;
            iterator end_it;
        };

    } // namespace _decl

    template<std_container T, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::any_of` instead")]]
#endif
    constexpr bool any(const T &container, const Pred &func);
    template<std_container T, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::any_of` instead")]]
#endif
    constexpr bool any(T &container, const Pred &func);
    template<std_container T, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::all_of` instead")]]
#endif
    constexpr bool all(const T &container, const Pred &func);
    template<std_container T, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::all_of` instead")]]
#endif
    constexpr bool all(T &container, const Pred &func);
    template<std_container T>
    constexpr bool contains(const T &container, const typename T::value_type &value);
    template<std_container T>
    constexpr bool contains(T &container, const typename T::value_type &value);
    template<std_container T, typename Func>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::for_each` instead")]]
#endif
    constexpr void for_each(const T &container, const Func &func);
    template<std_container T, typename Func>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::for_each` instead")]]
#endif
    constexpr void for_each(T &container, const Func &func);
    template<std_container T, typename Pred>
    constexpr typename T::value_type
    first_or_default(const T &container, const Pred &func,
                     const typename T::value_type &default_value = typename T::value_type());
    template<std_container T, typename Pred>
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
    template<std::size_t N, std_container T>
    constexpr std::array<typename T::value_type, N> to_array(T &container);
    template<std::size_t N, std_container T>
    constexpr std::array<typename T::value_type, N> to_array(const T &container);
    template<std_container T, typename Pred>
    constexpr size_t count_if(const T &container, const Pred &func);
    template<std_container T, typename Pred>
    constexpr size_t count_if(T &container, const Pred &func);
    template<std_container T, typename Compare = std::less<typename T::value_type>>
    constexpr typename T::value_type max(const T &container, const Compare &cmp = {});
    template<std_container T, typename Compare = std::less<typename T::value_type>>
    constexpr typename T::value_type max(T &container, const Compare &cmp = {});
    template<std_container T, typename Compare = more<typename T::value_type>>
    constexpr typename T::value_type min(const T &container, const Compare &cmp = {});
    template<std_container T, typename Compare = more<typename T::value_type>>
    constexpr typename T::value_type min(T &container, const Compare &cmp = {});

#if __cplusplus < 201703L
    template<std_container T, class Inserter = typename std::conditional<has_reserve<typename std::decay<T>::type>::value, std::back_insert_iterator<typename std::decay<T>::type>, std::insert_iterator<typename std::decay<T>::type>>::type, typename ...Args>
    constexpr void emplace_range(T &container, Args &&...args) = delete;
#else
    template<std_container T, class Inserter = typename std::conditional<_decl::has_reserve<typename std::decay<T>::type>::value, std::back_insert_iterator<typename std::decay<T>::type>, std::insert_iterator<typename std::decay<T>::type>>::type, typename ...Args>
    constexpr void emplace_range(T &container, Args &&...args);
#endif
    template<std_container T, class Inserter = typename std::conditional<has_reserve<typename std::decay<T>::type>::value, std::back_insert_iterator<typename std::decay<T>::type>, std::insert_iterator<typename std::decay<T>::type>>::type, typename Range>
    constexpr void emplace_range(T &container, Range &&range);
    template<class T, class Inserter = std::back_insert_iterator<T>>
    constexpr void emplace_range(T &container, std::initializer_list<typename T::value_type> list);

    template<std_container T, class Pred>
    constexpr typename std::enable_if<is_bool_predicate<Pred>::value, views::filter_ref_view<const T, Pred>>::type filter(const T &container, const Pred &func);
    template<std_container T, class Pred>
    constexpr typename std::enable_if<is_bool_predicate<Pred>::value, views::filter_view<typename std::decay<T>::type, Pred>>::type filter(T &&container, const Pred &func);
    template<typename T, size_t N, typename Pred, class AllocT = std::allocator<T>>
    constexpr typename std::enable_if<is_bool_predicate<Pred>::value, std::vector<T>>::type filter(const std::array<T, N> &array, const Pred &func);

    template<template<typename...> class T, typename... Args, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::transform` instead")]]
#endif
    constexpr views::transform<T<Args...>, Pred> transform(T<Args...> &container, const Pred &pred);
    template<template<typename...> class T, typename... Args, typename Pred>
#if __cplusplus >= 202002L && !(RANGED_NO_DEPRECATION_WARNINGS)
    [[deprecated("Preffer using `std::ranges::transform` instead")]]
#endif
    constexpr views::transform<const T<Args...>, Pred> transform(const T<Args...> &container, const Pred &pred);

    template<std_container T, std_container U>
    constexpr views::zip<T, U> zip(T &first, U &second);
    template<std_container T, std_container U>
    constexpr views::zip<const T, const U> zip(const T &first, const U &second);
    template<template<typename, std::size_t> class Ta, typename T1, typename T2, std::size_t N>
    constexpr Ta<std::tuple<T1, T2>, N> zip(Ta<T1, N> &first, Ta<T2, N> &second);
    template<template<typename, std::size_t> class Ta, typename T1, typename T2, std::size_t N>
    constexpr Ta<std::tuple<T1, T2>, N> zip(const Ta<T1, N> &first, const Ta<T2, N> &second);


#ifdef RANGED_IMPLEMENTATION


    template<std_container T, typename Pred>
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
    template<std_container T, typename Pred>
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

    template<std_container T, typename Pred>
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
    template<std_container T, typename Pred>
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
    template<std_container T>
    constexpr bool contains(const T &container, const typename T::value_type &value) {
#if __cplusplus >= 202002L
        return std::ranges::find(container, value) != container.end();
#else
        return std::find(container.begin(), container.end(), value) != container.end();
#endif
    }
    template<std_container T>
    constexpr bool contains(T &container, const typename T::value_type &value) {
#if __cplusplus >= 202002L
        return std::ranges::find(container, value) != container.end();
#else
        return std::find(container.begin(), container.end(), value) != container.end();
#endif
    }
    template<std_container T, typename Func>
    constexpr void for_each(const T &container, const Func &func) {
#if __cplusplus >= 202002L
        std::ranges::for_each(container, func);
#else
        for (const auto &element: container) {
            func(element);
        }
#endif
    }
    template<std_container T, typename Func>
    constexpr void for_each(T &container, const Func &func) {
#if __cplusplus >= 202002L
        std::ranges::for_each(container, func);
#else
        for (const auto &element: container) {
            func(element);
        }
#endif
    }
    template<std_container T, typename Pred>
    constexpr typename T::value_type first_or_default(const T &container, const Pred &func, const typename T::value_type &default_value) {
        for (const auto &item: container) {
            if (func(item))
                return item;
        }

        return default_value;
    }
    template<std_container T, typename Pred>
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
    template<std::size_t N, std_container T>
    constexpr std::array<typename T::value_type, N> to_array(T &container) {
        std::array<typename T::value_type, N> result{};
        const size_t iter_to = std::min(N, container.size());
        for (size_t i{0}; i < iter_to; ++i) {
            result[i] = container.at(i);
        }

        return result;
    }
    template<std::size_t N, std_container T>
    constexpr std::array<typename T::value_type, N> to_array(const T &container) {
        std::array<typename T::value_type, N> result{};
        const size_t iter_to = std::min(N, container.size());
        for (size_t i{0}; i < iter_to; ++i) {
            result[i] = container.at(i);
        }

        return result;
    }
    template<std_container T, typename Pred>
    constexpr size_t count_if(const T &container, const Pred &func) {
        size_t result{};
        for (const auto &item: container) {
            if (func(item))
                ++result;
        }

        return result;
    }
    template<std_container T, typename Pred>
    constexpr size_t count_if(T &container, const Pred &func) {
        size_t result{};
        for (const auto &item: container) {
            if (func(item))
                ++result;
        }

        return result;
    }
    template<class T, class Pred>
    constexpr
            typename std::enable_if<is_bool_predicate<Pred>::value, views::filter_ref_view<const T, Pred>>::type
            filter(const T &container, const Pred &func) {
        return views::filter_ref_view<const T, Pred>{container, func};
    }
    template<class T, class Pred>
    constexpr typename std::enable_if<is_bool_predicate<Pred>::value,
                                      views::filter_view<typename std::decay<T>::type, Pred>>::type
    filter(T &&container, const Pred &func) {
        return views::filter_view<typename std::decay<T>::type, Pred>{std::forward<T>(container), func};
    }
    template<typename T, size_t N, typename Pred, class AllocT>
    constexpr typename std::enable_if<is_bool_predicate<Pred>::value, std::vector<T>>::type
    filter(const std::array<T, N> &array, const Pred &func) {
        std::vector<T, AllocT> result(N);
#if __cplusplus >= 202002L
        auto [std::ignore, out_it] = std::ranges::copy_if(array, result.begin(), pred);
        result.resize(std::distance(result.begin(), out_it));
#elif __cplusplus >= 201304L
        size_t i{0};
        for (size_t j{0}; j < N; ++j) {
            if (func(array.at(j))) {
                result[i] = array.at(j);
                ++i;
            }
        }
        result.resize(i);
#else
        size_t i{0};
        for (size_t j{0}; j < N; ++j) {
            if (func(array.at(j)))
                result[i++] = array.at(j);
        }
        result.resize(i);
#endif
        return result;
    }
    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr views::transform<T<Args...>, Pred> transform(T<Args...> &container, const Pred &pred) {
        return views::transform<T<Args...>, Pred>{container, pred};
    }
    template<template<typename...> class T, typename... Args, typename Pred>
    constexpr views::transform<const T<Args...>, Pred> transform(const T<Args...> &container, const Pred &pred) {
        return views::transform<const T<Args...>, Pred>{container, pred};
    }
    template<std_container T, std_container U>
    constexpr views::zip<T, U> zip(T &first, U &second) {
        if (first.size() != second.size())
            throw std::runtime_error("Containers cannot have different size.");
        return views::zip<T, U>{first, second};
    }
    template<std_container T, std_container U>
    constexpr views::zip<const T, const U> zip(const T &first, const U &second) {
        if (first.size() != second.size()) throw std::runtime_error("Containers cannot have different size.");
        return views::zip<const T, const U>{first, second};
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
    template<std_container T, class Compare>
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
    template<std_container T, typename Compare>
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
    template<std_container T, typename Compare>
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
    template<std_container T, typename Compare>
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
#if __cplusplus >= 201703L
    template<class T, class Inserter, typename... Args>
    constexpr void emplace_range(T &container, Args &&...args) {
        static_assert(_decl::has_emplace_back<std::decay_t<T>>::value && !std::is_const_v<T>);
        (container.emplace_back(std::forward<Args>(args)), ...);
    }
#endif
    template<class T, class Inserter, typename Range>
    constexpr void emplace_range(T &container, Range &&range) {
        static_assert(!std::is_const<T>::value, "Container cannot be const.");
        std::copy(std::begin(std::forward<Range>(range)), std::end(std::forward<Range>(range)), Inserter{container});
    }

    template<class T, class Inserter>
    constexpr void emplace_range(T &container, std::initializer_list<typename T::value_type> list) {
        std::copy(list.begin(), list.end(), Inserter(container));
    }

#endif

} // namespace ranged

#endif // RANGED_H
