#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>

#ifndef PRINT_CHAR
#define PRINT_CHAR '*'
#endif

template<typename T, size_t... Is>
struct apply_zeroes_filtered {
    using type = T;
};

template<typename T, size_t I, size_t... Is>
struct apply_zeroes_filtered<T, I, Is...> {
    using type = typename apply_zeroes_filtered<typename T::template append<I>, Is...>::type;
};

template<typename T, size_t... Is>
struct apply_zeroes_filtered<T, 0, Is...> {
    using type = typename apply_zeroes_filtered<T, Is...>::type;
};

template<size_t N, size_t J, size_t I, size_t... Is>
struct nth_value {
    constexpr static auto value = (N == J) ? I : nth_value<N, J + 1, Is...>::value;
};

template<size_t N, size_t J, size_t I>
struct nth_value<N, J, I> {
    constexpr static auto value = I;
};

template<size_t N, size_t J, typename T, typename... Ts>
struct nth_type {
    using type = std::conditional_t<N == J, T, typename nth_type<N, J + 1, Ts...>::type>;
};

template<size_t N, size_t J, typename T>
struct nth_type<N, J, T> {
    using type = T;
};

template<typename JS, size_t size, size_t SUM, size_t A, size_t... Is>
struct sum_adjacent_h {
    using type = typename sum_adjacent_h<JS, size - 1, SUM + A, Is...>::type;
};

template<typename JS, size_t size, size_t SUM, size_t... Is>
struct sum_adjacent_h<JS, size, SUM, 0, Is...> {
    using type = std::conditional_t<
        SUM == 0,
        typename sum_adjacent_h<typename JS::template append<0>, sizeof...(Is), 0, Is...>::type,
        typename sum_adjacent_h<typename JS::template append<SUM, 0>, sizeof...(Is), 0, Is...>::type>;
};

template<typename JS, size_t SUM, size_t A>
struct sum_adjacent_h<JS, 1, SUM, A> {
    using type = typename JS::template append<SUM + A>;
};

template<typename JS, size_t SUM>
struct sum_adjacent_h<JS, 1, SUM, 0> {
    using type = typename JS::template append<SUM, 0>;
};

template<size_t... Is>
struct IS;

template<size_t... Is>
struct ISx {
    constexpr IS<1, Is...> operator*() const { return {}; }
    constexpr IS<1, Is...> operator!() const { return {}; }
};

template<size_t... Is>
struct IS {
    constexpr static auto size = sizeof...(Is);

    template<size_t... Js>
    static IS<Is..., Js...> concat_helper(const IS<Js...>&);

    template<typename JS>
    using concat = decltype(concat_helper(std::declval<JS>()));

    template<size_t... Js>
    using append = IS<Is..., Js...>;

    template<template<size_t...> typename T>
    using applied_to = T<Is...>;

    using filter_zeroes = typename apply_zeroes_filtered<IS<>, Is...>::type;

    template<size_t N>
    constexpr static auto nth = nth_value<N, 0, Is...>::value;

    friend void toStream(std::ostream& os, const IS&)
    {
        os << (std::string(Is ? Is * 2 : 2, Is ? PRINT_CHAR : '-') + ...) << "\n";
    }

    constexpr static auto first = nth<0>;

    constexpr ISx<Is...>   operator*() const { return {}; };
    constexpr ISx<Is...>   operator!() const { return {}; };
    constexpr IS<0, Is...> operator--() const { return {}; }
};

template<>
struct IS<> {
    constexpr static auto size = 0;

    template<size_t... Js>
    static IS<Js...> concat_helper(const IS<Js...>&);

    template<typename JS>
    using concat = decltype(concat_helper(std::declval<JS>()));

    template<size_t... Js>
    using append = IS<Js...>;

    template<template<size_t...> typename T>
    using applied_to = T<>;

    using filter_zeroes = IS<>;

    template<size_t N>
    constexpr static auto nth = 0;

    friend void toStream(std::ostream& os, const IS&) {}

    constexpr ISx<> operator*() const { return {}; }
    constexpr ISx<> operator!() const { return {}; }
    constexpr IS<0> operator--() const { return {}; }
};

template<typename JS>
struct sum_adjacent {
    template<size_t... Is>
    static typename sum_adjacent_h<IS<>, JS::size, 0, Is...>::type sum_helper(const IS<Is...>&);

    using type = decltype(sum_helper(std::declval<JS>()));
};

template<typename... Rs>
struct Rows {
    constexpr static auto size = sizeof...(Rs);

    template<typename R>
    using add = Rows<Rs..., R>;

    template<size_t N>
    using nth_type = typename nth_type<N, 0, Rs...>::type;

    static auto print()
    {
        (toStream(std::cout, Rs()), ...);
        std::cout << "\n";
        return Rows();
    }
};

template<>
struct Rows<> {
    constexpr static auto size = 0;

    template<typename R>
    using add = Rows<R>;
};

template<size_t COL, typename Rs>
struct x_col {
    template<size_t... Is>
    constexpr static auto col_helper(const std::index_sequence<Is...>&)
    {
        return IS<Rs::template nth_type<Is>::template nth<COL>...>();
    }

    using sequence      = std::make_index_sequence<Rs::size>;
    using type          = typename sum_adjacent<decltype(col_helper(std::declval<sequence>()))>::type::filter_zeroes;
    using filtered_only = typename decltype(col_helper(std::declval<sequence>()))::filter_zeroes;
    using unfilt        = decltype(col_helper(std::declval<sequence>()));
};

template<typename Rs, typename Cs = Rows<>, typename Es = Rows<>, bool Valid = true>
struct NonoPuzzle {
    constexpr static auto valid = Valid;

    template<size_t... Is>
    constexpr static auto c = NonoPuzzle<typename Rs::template add<IS<Is...>>, Cs, Es, Valid>();

    template<size_t... Is, typename T>
    constexpr static auto r(const T&)
    {
        using row_summed          = typename sum_adjacent<T>::type::filter_zeroes;
        constexpr auto this_valid = std::is_same_v<IS<Is...>, row_summed> && valid;
        static_assert(this_valid);
        return NonoPuzzle<Rs, typename Cs::template add<IS<Is...>>, typename Es::template add<T>, this_valid>();
    }

    auto print() const { return Es::print(); }

    auto print_types() const { return Es::print_types(); }

    template<size_t I>
    constexpr static auto col_valid()
    {
        return std::is_same_v<typename x_col<I, Rs>::filtered_only, typename x_col<I, Es>::type>;
    }

    template<size_t... Is>
    constexpr auto cols_valid(const std::index_sequence<Is...>&) const
    {
        constexpr auto cols_valid = (col_valid<Is>() && ...);
        static_assert(cols_valid);
    }

    constexpr auto check() const
    {
        static_assert(valid);
        cols_valid(std::make_index_sequence<Rs::template nth_type<0>::size>());
        return *this;
    }
};

constexpr struct Nono {
    template<size_t... Is>
    constexpr static auto c = NonoPuzzle<Rows<IS<Is...>>>();
} NONO;

constexpr IS<> I;
