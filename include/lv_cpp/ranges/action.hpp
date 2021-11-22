#ifndef __ACTION_HPP__
#define __ACTION_HPP__

#include <ranges>
#include <algorithm>

namespace leviathan::action
{
    template <typename Lambda>
    class bind_back_impl
    {
    public:
        constexpr bind_back_impl(Lambda lambda) : m_func(std::move(lambda))
        {
        }

        template <typename View>
        constexpr friend auto operator|(View v, bind_back_impl&& back)
        {
            return std::invoke(back.m_func, v);
        }

    private:
        Lambda m_func;
    };

    template <typename Lambda>
    bind_back_impl(Lambda&&) -> bind_back_impl<Lambda>;

    template <typename Fn, typename... Args>
    constexpr auto bind_back(Fn fn, Args... args)
    {
        return bind_back_impl{[fn, ... args = std::move(args)]<typename... Ts>(Ts... ts) mutable
        {
            return std::invoke(fn, std::forward<Ts>(ts)..., std::move(args)...);
        }};
    }

#define ACTION_GENERATOR(object)                                                \
    [[maybe_unused]]                                                            \
    constexpr auto object = []<typename... Args>(Args&&... args)                \
    {                                                                           \
        return bind_back(std::ranges:: object , std::forward<Args>(args)...);   \
    }

    ACTION_GENERATOR(for_each);
    ACTION_GENERATOR(max_element);
    ACTION_GENERATOR(max);
    ACTION_GENERATOR(count);
    ACTION_GENERATOR(count_if);
    ACTION_GENERATOR(sort);
    ACTION_GENERATOR(unique);

#undef ACTION_GENERATOR
}


#endif