#include <iostream>
#include <functional>

template <typename Adaptor, typename... Args>
concept adaptor_invocable = requires 
{
    std::declval<Adaptor>()(std::declval<Args>()...);
};

template <typename Adaptor, typename... Args> struct partial;

template <typename Lhs, typename Rhs> struct pipe;

// simple pipeline
struct range_adaptor_closure
{
    template <typename Self, typename Range>
    requires std::derived_from<std::remove_cvref_t<Self>, range_adaptor_closure> && adaptor_invocable<Self, Range>
    friend constexpr auto operator|(Range&& r, Self&& self)
    { return std::forward<Self>(self)(std::forward<Range>(r)); }

    template <typename Lhs, typename Rhs>
    requires std::derived_from<Lhs, range_adaptor_closure> && std::derived_from<Rhs, range_adaptor_closure>
    friend constexpr auto operator|(Lhs lhs, Rhs rhs)
    { return pipe<Lhs, Rhs>{ std::move(lhs), std::move(rhs)}; }
};

template <typename Derived>
struct range_adaptor
{
    template <typename... Args> 
    // requires adaptor_invocable<Derived, Args...>
    constexpr auto operator()(Args&&... args) const 
    { return partial<Derived, std::decay_t<Args>...>{ std::forward<Args>(args)... }; }
};

template <typename Adaptor, typename... Args>
struct partial : range_adaptor_closure
{
    std::tuple<Args...> m_args;

    constexpr partial(Args... args) : m_args(std::move(args)...) { }

    template <typename Range>
    requires adaptor_invocable<Adaptor, Range, const Args&...>
    constexpr auto operator()(Range&& r) const&
    {
        auto forwarder = [&r](const auto&... args) {
            return Adaptor{}(std::forward<Range>(r), args...);
        };
        return std::apply(forwarder, m_args);
    }

    template <typename Range>
    requires adaptor_invocable<Adaptor, Range, Args...>
    constexpr auto operator()(Range&& r) &&
    {
        auto forwarder = [&r](auto&... args) {
            return Adaptor{}(std::forward<Range>(r), std::move(args)...);
        };
        return std::apply(forwarder, m_args);
    }

    template <typename Range>
    constexpr auto operator()(Range&& r) const&& = delete;
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



int main(int argc, char const *argv[])
{
    
    return 0;
}
