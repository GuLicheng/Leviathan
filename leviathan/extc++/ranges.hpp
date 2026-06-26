#pragma once

#include <ranges>
#include <concepts>
#include <tuple>
#include <utility>
#include <functional>
#include <type_traits>
#include <assert.h>
#include <variant>
#include <leviathan/extc++/concepts.hpp>
#include <leviathan/extc++/functional.hpp>
#include <leviathan/type_caster.hpp>
// #include <leviathan/math/int128.hpp>

// Some utils
namespace cpp::ranges::detail
{

using cpp::meta::maybe_const_t;
using cpp::meta::simple_view;

template <typename F, typename Tuple>
constexpr auto tuple_transform(F&& f, Tuple&& tuple)
{
    return std::apply([&]<typename... Ts>(Ts&&... elements) {
        return meta::tuple_or_pair<std::invoke_result_t<F&, Ts>...>(std::invoke(f, (Ts&&) elements)...);
    }, (Tuple&&) tuple);
}

template <typename F, typename Tuple>
constexpr void tuple_for_each(F&& f, Tuple&& tuple)
{
    std::apply([&]<typename... Ts>(Ts&&... elements) {
        (std::invoke(f, (Ts&&) elements), ...);
    }, (Tuple&&) tuple);
} 

template <typename... Ts>
struct last_element : std::type_identity<decltype((std::type_identity<Ts>{}, ...))> { };

template <typename... Ts>
using last_element_t = typename last_element<Ts...>::type;

template <bool HasType, typename T>
struct has_typedef_name_of_iterator_category { };

template <typename T>
struct has_typedef_name_of_iterator_category<true, T> 
{
    using iterator_category = T;
};

// some iterator may not have iterator_category so we use follow meta as helper
// std::derived_from<void, tag> will be false for any tag in [input/forward/bidirectional/random_access]iterator_tag
template <typename Iter, bool HasIteratorCategory>
struct iter_category_impl : std::type_identity<void> { };

template <typename Iter>
struct iter_category_impl<Iter, true> : std::type_identity<typename std::iterator_traits<Iter>::iterator_category> { };

template <typename Iter>
concept has_iterator_category = requires 
{
    typename std::iterator_traits<Iter>::iterator_category;
};

template <typename T>
using iter_category_t = typename iter_category_impl<T, has_iterator_category<T>>::type;

template <typename Base>
consteval auto simple_iterator_concept()
{
    if constexpr (std::ranges::random_access_range<Base>)
        return std::random_access_iterator_tag();
    else if constexpr (std::ranges::bidirectional_range<Base>)
        return std::bidirectional_iterator_tag();
    else if constexpr (std::ranges::input_range<Base>)
        return std::forward_iterator_tag();
    else    
        return std::input_iterator_tag();
}

// template <typename T>
// constexpr auto to_signed_like(T x) 
// {
//     if constexpr (!std::integral<T>)
//         return std::iter_difference_t<T>();
//     else if constexpr (sizeof(std::iter_difference_t<T>) > sizeof(T))
//         return std::iter_difference_t<T>(x);
//     else if constexpr (sizeof(std::ptrdiff_t) > sizeof(T))
//         return std::ptrdiff_t(x);
//     else if constexpr (sizeof(long long) > sizeof(T))
//         return (long long)(x);
//     else
//         return int128_t(x);  // return __max_diff_type(x);
// }

template<typename T>
using iota_diff_t = decltype(to_signed_like(std::declval<T>()));

}

namespace cpp::ranges
{

// https://open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2387r3.html
template <typename F>
class closure : public std::ranges::range_adaptor_closure<closure<F>>
{
    F f;

public:
    constexpr closure(F f) : f(std::move(f)) {}

    template <std::ranges::viewable_range R>
        requires std::invocable<F const &, R>
    auto constexpr operator()(R &&r) const
    {
        return f(std::forward<R>(r));
    }
};

template <typename F>
class adaptor
{
    F f;

public:
    constexpr adaptor(F f) : f(std::move(f)) {}

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
    {
        if constexpr (std::invocable<F const &, Args...>)
        {
            return f(std::forward<Args>(args)...);
        }
        else
        {
            return closure(std::bind_back(f, std::forward<Args>(args)...));
        }
    }
};

template <typename F, typename... Args>
class partial : public std::ranges::range_adaptor_closure<partial<F, Args...>>
{
    std::tuple<Args...> m_args;
    F m_f;

public:

    template <typename... Ts>
    constexpr partial(F closure, Ts&&... ts) : m_args((Ts&&) ts...) 
    { }

    template <typename Self, std::ranges::viewable_range R>
    constexpr auto operator()(this Self&& self, R&& r)
    {
        return std::invoke(
            ((Self&&)self).m_f,
            ((Self&&)self).m_args,
            (R&&) r
        );
    }
};

/**
 * @brief Collects elements from a range into an associative container.
 *  For items with same key, it will sum their values.
 *   
 *     std::multimap<std::string, double> ("key1" : 1.0, "key1" : 2.0) 
 *  -> std::map<std::string, double>      ("key1" : 3.0)
 */
template <typename AssociateContainer>
inline constexpr adaptor collect = []<typename... Args>(Args&&... args) static
{
    auto fn = []<typename Tuple, typename R>(Tuple&& t, R&& r) static
    {
        auto map = std::make_from_tuple<AssociateContainer>((Tuple&&)t);

        for (auto&& [key, value] : r)
        {
            // The multimap will not provide `try_emplace`, it will cause compiler error.
            auto [pos, ok] = map.try_emplace(key, value);

            if (!ok)
            {
                pos->second += value;
            }
        }

        return map;
    };

    return partial<decltype(fn), std::decay_t<Args>...>(std::move(fn), (Args&&)args...);
};

// See Python.collections.Counter
template <typename AssociateContainer>
inline constexpr adaptor counter = []<typename... Args>(Args&&... args) static
{
    using MappedType = typename AssociateContainer::mapped_type;
    static_assert(std::integral<MappedType>);

    auto fn = []<typename Tuple, typename R>(Tuple&& t, R&& r) static
    {
        auto map = std::make_from_tuple<AssociateContainer>((Tuple&&)t);

        for (auto&& value : r)
        {
            auto [pos, ok] = map.try_emplace(value, (MappedType)1);

            if (!ok)
            {
                pos->second++;
            }
        }

        return map;
    };

    return partial<decltype(fn), std::decay_t<Args>...>(std::move(fn), (Args&&)args...);
};

}  // namespace cpp::ranges

namespace cpp::ranges
{

inline constexpr struct
{
    template <typename T>
    static constexpr decltype(auto) operator()(T&& t)
    {
        return *std::ranges::begin(t);
        // return t.front();
    }
} front;

inline constexpr struct
{
    template <typename T>
    static constexpr decltype(auto) operator()(T&& t)
    {
        return *(--std::ranges::end(t)); // sentinel ?
        // return t.back();
    }
} back;

}  // namespace cpp

namespace cpp::ranges::views
{

using namespace std::views;

template <typename T>
inline constexpr closure as = []<typename R>(R&& r) static
{
    return (R&&)r | transform(cpp::cast<T>);
};

inline constexpr closure indirect = []<typename R>(R&& r) static
{
    return (R&&)r | transform(indirection);
};

inline constexpr closure format = []<typename R>(R&& r) static
{
    return (R&&)r | transform(to_string) | cache_latest;
};

inline constexpr closure head = []<typename R>(R&& r) static
{
    return (R&&)r | transform(front); 
};

inline constexpr closure tail = []<typename R>(R&& r) static
{
    return (R&&)r | transform(back);
};

inline constexpr closure unique = []<typename R>(R&& r) static
{
    return (R&&)r | std::views::chunk_by(std::ranges::equal_to()) | head;
};

inline constexpr closure to_lower = []<typename R>(R&& r) static
{
    return (R&&)r | transform([]<typename CharT>(CharT c) static { return (CharT)::tolower(c); });
};

inline constexpr closure to_upper = []<typename R>(R&& r) static
{
    return (R&&)r | transform([]<typename CharT>(CharT c) static { return (CharT)::toupper(c); });
};

inline constexpr closure split_line = []<typename R>(R&& r) static
{
    return (R&&)r | split('\n') | transform([](auto&& x) static { return std::string_view(x); });
};

inline constexpr closure cycle = []<typename R>(R&& r) static
{
    return repeat((R&&)r) | join;
};

}  // namespace cpp::ranges::views

namespace cpp::ranges::views
{

inline constexpr auto apply = []<typename F>(F&& f) static
{
    auto fn = [f = (F&&)f]<meta::tuple_like TupleLike>(TupleLike&& t) 
    {
        return std::apply(f, (TupleLike&&)t);
    };
    return transform(std::move(fn));
};

inline constexpr auto pair_transform = []<typename F1, typename F2>(F1&& f1, F2&& f2) static
{
    return transform(make_tuple_callables((F1&&)f1, (F2&&)f2));

    // auto transfer = [f1 = (F1&&)f1, f2 = (F2&&)f2]<meta::pair_like PairLike>(PairLike&& x) 
    // {
    //     return std::make_pair(
    //         std::invoke(f1, std::get<0>((PairLike&&)x)),
    //         std::invoke(f2, std::get<1>((PairLike&&)x))
    //     );
    // };

    // return transform(std::move(transfer));
};

inline constexpr auto compose = []<typename... Fs>(Fs&&... fs) static
{
    // The order in ranges is reversed
    return transform(projection((Fs&&)fs...)); 
};

inline constexpr auto transform_join = []<typename F>(F&& f) static
{
    return transform((F&&)f) | join;
};

inline constexpr auto transform_join_with = []<typename F, typename Delimiter>(F&& f, Delimiter&& d) static 
{
    return transform((F&&)f) | join_with((Delimiter&&)d);
};

inline constexpr auto replace_if = []<typename Pred, typename T>(Pred&& pred, T&& new_value) static 
{
    auto fn = [pred = (Pred&&)pred, new_value = (T&&)new_value](auto&& x) 
    {
        return pred(x) ? new_value : x;
    };
    return transform(fn);
};

inline constexpr auto replace = []<typename T>(T&& old_value, T&& new_value) static 
{
    auto fn = [old_value = (T&&)old_value, new_value = (T&&)new_value](auto&& x) 
    {
        return x == old_value ? new_value : x;
    };
    return transform(fn);
};

inline constexpr auto remove = []<typename T>(T&& value) static 
{
    return filter([value = (T&&)value](auto&& x) { return x != value; });
};

inline constexpr auto remove_if = []<typename Pred>(Pred&& pred) static 
{
    return filter([pred = (Pred&&)pred](auto&& x) { return !pred(x); });
};

}  // namespace cpp::ranges::views


namespace cpp::views
{
    using namespace cpp::ranges::views;
}

#include <algorithm>

namespace cpp::action
{

// inline constexpr cpp::ranges::adaptor for_each = 
//     []<std::ranges::viewable_range R, 
//     typename F, typename Proj = std::identity>(R &&r, F f, Proj proj = {}) static -> R
// {
//     std::ranges::for_each((R &&)r, std::move(f), std::move(proj));
//     return (R &&)r;
// };

inline constexpr cpp::ranges::closure sort = []<std::ranges::viewable_range R>(R&& r) static
{
    std::ranges::sort(r);
    return (R&&)r;
};

inline constexpr cpp::ranges::adaptor for_each = 
    []<std::ranges::viewable_range R, typename... Args>(R &&r, Args... args) static 
{
    return std::ranges::for_each((R &&)r, std::move(args...));
};

template <auto FunctionObject>
inline constexpr cpp::ranges::closure function_closure = []<std::ranges::viewable_range R>(R&& r) static
{
    return FunctionObject((R&&)r);
};

inline constexpr auto max = function_closure<std::ranges::max>;
inline constexpr auto min = function_closure<std::ranges::min>;
inline constexpr auto size = function_closure<std::ranges::size>;

}
