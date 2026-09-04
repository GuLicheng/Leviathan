#include <iostream>
#include <functional>
#include <ranges>
#include <print>
#include <winnow/all.hpp>

template <typename Lhs, typename Rhs> struct pipe;

struct adaptor_closure;

template <typename T>
concept parser = std::derived_from<std::decay_t<T>, winnow::detail::parser_interface>;

template <typename Adaptor, typename... Args>
concept adaptor_invocable = requires 
{
    std::declval<Adaptor>()(std::declval<Args>()...);
};

template <typename T>
concept parser_adaptor_closure = std::derived_from<std::decay_t<T>, adaptor_closure>
                              && std::move_constructible<std::decay_t<T>>
                              && std::constructible_from<std::decay_t<T>, T>;

struct adaptor_closure
{   
    template <parser_adaptor_closure Self, typename Stream>
        requires adaptor_invocable<Self, Stream>
    friend constexpr auto operator|(Stream&& p, Self&& self)
    { 
        return static_cast<Self&&>(self)(static_cast<Stream&&>(p));
    }

    template <parser_adaptor_closure Lhs, parser_adaptor_closure Rhs>
    friend constexpr auto operator|(Lhs&& lhs, Rhs&& rhs)
    { 
        return pipe<std::decay_t<Lhs>, std::decay_t<Rhs>>(static_cast<Lhs&&>(lhs), static_cast<Rhs&&>(rhs)); 
    }
};

// R | (A | B), the A and B and (A | B) are all adaptor_closure instances
template <typename Lhs, typename Rhs>
struct pipe : adaptor_closure
{
    [[no_unique_address]] Lhs lhs;
    [[no_unique_address]] Rhs rhs;

    template <typename L, typename R>
    constexpr pipe(L&& lhs, R&& rhs) : lhs((L&&)lhs), rhs((R&&)rhs) { }

    template <typename Self, typename Other>
    constexpr auto operator()(this Self&& self, Other&& other) 
    {
        return static_cast<Self&&>(self).rhs(
            static_cast<Self&&>(self).lhs(
                static_cast<Other&&>(other)
            )
        );
    }
};

template <typename F>
class closure : public adaptor_closure
{
    F f;

public:

    template <typename F2>
    constexpr closure(F2&& f) : f(static_cast<F2&&>(f)) { }

    template <typename Self, typename R>
    auto constexpr operator()(this Self&& self, R&& r)
    {
        return static_cast<Self&&>(self).f(static_cast<R&&>(r));
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


int main(int argc, char const *argv[])
{
    std::println("Hello, World!");
    return 0;
}
