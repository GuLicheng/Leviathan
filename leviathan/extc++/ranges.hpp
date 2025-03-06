#pragma once

#include "concepts.hpp"
#include <ranges>

namespace leviathan
{

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

}  // namespace leviathan

namespace leviathan::action
{
    
}