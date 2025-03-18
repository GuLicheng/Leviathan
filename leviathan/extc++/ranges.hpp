#pragma once

#include "concepts.hpp"
#include "concat.hpp"
#include <ranges>

namespace leviathan::ranges
{

namespace detail
{

template <bool Reverse, typename... Callables>
class adaptors
{
    static constexpr size_t size = sizeof...(Callables);

    [[no_unique_address]] std::tuple<Callables...> m_callables;

    template <size_t N, typename Tuple, typename... Args>
    constexpr static auto call(Tuple&& tuple_like, Args&&... args)
    {
        if constexpr (Reverse)
        {
            if constexpr (N == 0)
            {
                return std::get<0>((Tuple&&)tuple_like)((Args&&)args...);
            }
            else
            {
                return std::get<N>((Tuple&&)tuple_like)(call<N - 1>((Tuple&&)tuple_like, (Args&&)args...));
            }
        }
        else
        {
            if constexpr (N == size - 1)
            {
                return std::get<N>((Tuple&&)tuple_like)((Args&&)args...);
            }
            else
            {
                return std::get<N>((Tuple&&)tuple_like)(call<N + 1>((Tuple&&)tuple_like, (Args&&)args...));
            }
        }
    }

public:

    template <typename... Callables1>
    constexpr adaptors(Callables1&&... callables) : m_callables((Callables1&&)callables...) 
    { }

    template <typename Self, typename... Args>
    constexpr auto operator()(this Self&& self, Args&&... args)
    {
        constexpr auto idx = Reverse ? size - 1 : 0;
        return call<idx>(((Self&&)self).m_callables, (Args&&)args...);
    }
};  

}  // namespace detail

inline constexpr struct 
{
    template <typename... Fns>
    constexpr static auto operator()(Fns&&... fns)
    {
        return detail::adaptors<false, std::decay_t<Fns>...>((Fns&&)fns...);
    }
} composition;

inline constexpr struct 
{
    template <typename... Fns>
    constexpr static auto operator()(Fns&&... fns)
    {
        return detail::adaptors<true, std::decay_t<Fns>...>((Fns&&)fns...);
    }
} projection;

#if 0
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2387r1.html
template <typename F>
class closure : public std::ranges::range_adaptor_closure<closure<F>>
{
    F f;

public:
    constexpr closure(F f) : f(f) {}

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
    constexpr adaptor(F f) : f(f) {}

    template <typename... Args>
    constexpr auto operator()(Args &&...args) const
    {
        if constexpr (std::invocable<F const &, Args...>)
        {
            return f(std::forward<Args>(args)...);
            // return std::invoke(f, std::forward<Args>(args)...);
        }
        else
        {
            return closure(std::bind_back(f, std::forward<Args>(args)...));
        }
    }
};

// inline constexpr closure join
//     = []<viewable_range R> requires /* ... */
//       (R&& r) {
//         return join_view(FWD(r));
//       };
      
// inline constexpr adaptor transform
//     = []<viewable_range R, typename F> requires /* ... */
//       (R&& r, F&& f){
//         return transform_view(FWD(r), FWD(f));
//       };

#endif

inline constexpr struct 
{
    template <typename T>
    static constexpr auto operator()(T&& t) 
    {
        return std::format("{}", (T&&)t);
    }
} to_string;

inline constexpr struct 
{
    template <typename It>
    static constexpr decltype(auto) operator()(It&& it)
    {
        return *it;
    } 
} indirection;

inline constexpr struct 
{
    template <typename T>
    static constexpr T* operator()(T& t)
    {
        return std::addressof(t);
    }
} addressof;

}  // namespace leviathan

namespace leviathan::ranges::views
{

using namespace std::views;

inline constexpr closure indirect = []<typename R>(R&& r)
{
    return (R&&)r | transform(indirection);
};

inline constexpr closure format = []<typename R>(R&& r)
{
    return (R&&)r | transform(to_string);
};

inline constexpr auto compose = []<typename... Fs>(Fs&&... fs)
{
    // The order in ranges is reversed
    return transform(projection((Fs&&)fs...)); 
};

inline constexpr auto transform_join = []<typename F>(F&& f) 
{
    return transform((F&&)f) | join;
};

inline constexpr auto transform_join_with = []<typename F, typename Delimiter>(F&& f, Delimiter&& d) 
{
    return transform((F&&)f) | join_with((Delimiter&&)d);
};

inline constexpr auto replace_if = []<typename Pred, typename T>(Pred&& pred, T&& new_value)
{
    auto fn = [pred = (Pred&&)pred, new_value = (T&&)new_value](auto&& x) 
    {
        return pred(x) ? new_value : x;
    };
    return transform(fn);
};

inline constexpr auto replace = []<typename T>(T&& old_value, T&& new_value)
{
    auto fn = [old_value = (T&&)old_value, new_value = (T&&)new_value](auto&& x) 
    {
        return x == old_value ? new_value : x;
    };
    return transform(fn);
};

inline constexpr auto remove = []<typename T>(T&& value)
{
    return filter([value = (T&&)value](auto&& x) { return x != value; });
};

inline constexpr auto remove_if = []<typename Pred>(Pred&& pred)
{
    return filter([pred = (Pred&&)pred](auto&& x) { return !pred(x); });
};

}  // namespace leviathan::ranges::views


namespace leviathan::views
{
    using namespace leviathan::ranges::views;
}