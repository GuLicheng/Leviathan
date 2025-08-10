#pragma once

#include <format>
#include <functional>

namespace cpp
{
    
inline constexpr struct
{
    template <typename T>  
    static constexpr std::string operator()(T&& value, std::string_view fmt = "{}") 
    {
        return std::vformat(fmt, std::make_format_args(value));
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

inline constexpr struct 
{
    template <typename It>
    static constexpr decltype(auto) operator()(It&& it)
    {
        return *it;
    } 
} indirection;

} // namespace cpp


