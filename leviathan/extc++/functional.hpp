#pragma once

#include <format>
#include <functional>
#include <leviathan/type_caster.hpp>

namespace cpp::detail
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

template <typename... Callables>
class tuple_callables 
{
    std::tuple<Callables...> m_callables;

    template <size_t... Idx, typename FunctionTuple, typename ArgTuple>
    static constexpr auto call(std::index_sequence<Idx...>, FunctionTuple&& fns, ArgTuple&& args)
    {
        return std::make_tuple(
            std::invoke(std::get<Idx>((FunctionTuple&&)fns), std::get<Idx>((ArgTuple&&)args))...
        );
    }

public:

    template <typename... Fns>
    constexpr tuple_callables(Fns&&... fns) : m_callables((Fns&&) fns...) { }

    template <typename Self,  meta::tuple_like TupleLike>
    constexpr auto operator()(this Self&& self, TupleLike&& args)
    {
        return call(std::make_index_sequence<sizeof...(Callables)>(), ((Self&&)self).m_callables, (TupleLike&&)args);
    }
};

}

namespace cpp
{
    
inline constexpr struct
{
    template <typename... Callables>
    static constexpr auto operator()(Callables&&... callables)
    {
        return detail::tuple_callables<std::decay_t<Callables>...>((Callables&&) callables...);
    }
} make_tuple_callables;

inline constexpr struct
{
    template <typename T>
    static constexpr std::string operator()(T&& value) 
    {
        return std::format("{}", value);
    }
} to_string;

inline constexpr struct 
{
    template <typename T>
    static constexpr T* operator()(T& t)
    {
        return std::addressof(t);
    }
} addressof;

// composition(f, g)(x) == f(g(x))
inline constexpr struct 
{
    template <typename... Fns>
    constexpr static auto operator()(Fns&&... fns)
    {
        return detail::adaptors<false, std::decay_t<Fns>...>((Fns&&)fns...);
    }
} composition;

// projection(p, f)(xs...) == f(p(xs)...)
inline constexpr struct 
{
    template <typename... Fns>
    constexpr static auto operator()(Fns&&... fns)
    {
        return detail::adaptors<true, std::decay_t<Fns>...>((Fns&&)fns...);
    }
} projection;

inline constexpr struct 
{
    template <typename It>
    static constexpr decltype(auto) operator()(It&& it)
    {
        return *it;
    } 
} indirection;

} // namespace cpp

namespace cpp
{

struct iter_hash_key_equal
{
    using is_transparent = void;

    template <typename I>
    static constexpr auto operator()(I it)
    {
        using ValueType = std::iter_value_t<I>;
        return std::hash<ValueType>()(*it);
    }

    template <typename I>
    static constexpr auto operator()(I it1, I it2)
    {
        using ValueType = std::iter_value_t<I>;
        return std::ranges::equal_to()(*it1, *it2);
    }
};

}
