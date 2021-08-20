#ifndef __ACTION_HPP__
#define __ACTION_HPP__

namespace leviathan::action
{
    template <typename Lambda>
    class bind_back_impl
    {
    public:
        constexpr bind_back_impl(Lambda lambda) : m_func(std::move(lambda))
        {
        }

        template <typename Range>
        constexpr friend auto operator|(Range &&rg, bind_back_impl &&back)
        {
            return std::invoke(back.m_func, std::ranges::begin(rg), std::ranges::end(rg));
        }

        template <typename Range>
        constexpr friend auto operator|(Range &&rg, bind_back_impl &back)
        {
            return std::invoke(back.m_func, std::ranges::begin(rg), std::ranges::end(rg));
        }

        template <typename Range>
        constexpr friend auto operator|(Range &&rg, const bind_back_impl &back)
        {
            return std::invoke(back.m_func, std::ranges::begin(rg), std::ranges::end(rg));
        }

    private:
        Lambda m_func;
    };

    template <typename Lambda>
    bind_back_impl(Lambda &&) -> bind_back_impl<Lambda>;

    template <typename Fn, typename... Args>
    constexpr auto bind_back(Fn &&fn, Args &&...args)
    {
        return bind_back_impl{[fn = std::forward<Fn>(fn), ... args = std::forward<Args>(args)]<typename... Ts>(Ts... ts) mutable 
        {
            return std::invoke(std::forward<Fn>(fn), std::forward<Ts>(ts)..., std::forward<Args>(args)...);
        }};
    }

}

/*
    constexpr int arr[] = {1, 2, 3};
    auto res = arr 
        | std::views::reverse
        | std::views::filter([](int x) { return x & 1; })
        // | bind_back(std::ranges::for_each, [](int x) { std::cout << x << ' '; })
        | bind_back(std::ranges::binary_search, 1, std::greater<>());
        ;
    console::write_line(res);  // true
*/

#endif