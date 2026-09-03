#include <iostream>
#include <functional>
#include <ranges>

template <typename Adaptor, typename... Args> struct partial;

template <typename Lhs, typename Rhs> struct pipe;

template <typename Adaptor, typename... Args>
concept adaptor_invocable = requires 
{
    std::declval<Adaptor>()(std::declval<Args>()...);
};

template <typename Derived, typename Base>
concept remove_cvref_derived_from = std::derived_from<std::remove_cvref_t<Derived>, Base>;

// simple pipeline
struct range_adaptor_closure
{
    template <remove_cvref_derived_from Self, typename Range>
        requires adaptor_invocable<Self, Range>
    friend constexpr auto operator|(Range&& r, Self&& self)
    { 
        return std::forward<Self>(self)(std::forward<Range>(r)); 
    }

    template <remove_cvref_derived_from<range_adaptor_closure> Lhs, remove_cvref_derived_from<range_adaptor_closure> Rhs>
    friend constexpr auto operator|(Lhs&& lhs, Rhs&& rhs)
    { 
        return pipe<std::decay_t<Lhs>, std::decay_t<Rhs>>{ std::forward<Lhs>(lhs), std::forward<Rhs>(rhs)}; 
    }
};

template <typename Derived>
struct range_adaptor
{
    template <typename... Args> 
        requires adaptor_invocable<Derived, Args...>
    constexpr auto operator()(Args&&... args) const 
    { return partial<Derived, std::decay_t<Args>...>{ 0, std::forward<Args>(args)... }; }
};

template <typename Adaptor, typename... Args>
struct partial : range_adaptor_closure
{
    std::tuple<Args...> m_args;

    template <typename... Ts>
    constexpr partial(int, Ts&&... args) : m_args(std::forward<Ts>(args)...) { }

    template <typename Self, typename Range>
        requires adaptor_invocable<Adaptor, Range, const Args&...>
    constexpr auto operator()(this Self&& self, Range&& r) const
    {
        auto forwarder = [&r]<typename... Ts>(Ts&&... args) {
            return Adaptor{}(std::forward<Range>(r), std::forward<Ts>(args)...);
        };
        return std::apply(forwarder, std::forward_like<Self>(self.m_args));
    }

    // template <typename Range>
    // requires adaptor_invocable<Adaptor, Range, const Args&...>
    // constexpr auto operator()(Range&& r) const&
    // {
    //     auto forwarder = [&r](const auto&... args) {
    //         return Adaptor{}(std::forward<Range>(r), args...);
    //     };
    //     return std::apply(forwarder, m_args);
    // }

    // template <typename Range>
    // requires adaptor_invocable<Adaptor, Range, Args...>
    // constexpr auto operator()(Range&& r) &&
    // {
    //     auto forwarder = [&r](auto&... args) {
    //         return Adaptor{}(std::forward<Range>(r), std::move(args)...);
    //     };
    //     return std::apply(forwarder, m_args);
    // }

    // template <typename Range>
    // constexpr auto operator()(Range&& r) const&& = delete;
};

template <typename Lhs, typename Rhs, typename Range>
concept pipe_invocable = requires 
{
    std::declval<Rhs>()(std::forward<Lhs>()(std::declval<Range>()));
};

template <typename Lhs, typename Rhs>
struct pipe : range_adaptor_closure
{
    [[no_unique_address]] Lhs m_lhs;
    [[no_unique_address]] Rhs m_rhs;

    constexpr pipe(Lhs lhs, Rhs rhs) : m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) { }

    template <typename Range>
    requires pipe_invocable<const Lhs&, const Rhs&, Range>
    constexpr auto operator()(Range&& r) const&
    { return m_rhs(m_lhs(std::forward<Range>(r))); }

    template <typename Range>
    requires pipe_invocable<Lhs, Rhs, Range>
    constexpr auto operator()(Range&& r) &&
    { return std::move(m_rhs)(std::move(m_lhs)(std::forward<Range>(r))); }

    template <typename Range>
    constexpr auto operator()(Range&& r) const&& = delete;

};

template <typename F>
class closure : public range_adaptor_closure<closure<F>>
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


int main(int argc, char const *argv[])
{
    return 0;
}
