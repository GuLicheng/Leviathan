#pragma once

#include "../bind_back.hpp"

#include <ranges>
#include <algorithm>


namespace leviathan::action
{

    template <typename Callable>
    class action_wrapper
    {
        Callable m_call;

        template <std::ranges::view View>
        constexpr friend auto operator|(View&& vw, action_wrapper&& rhs) 
        {
            return std::invoke(std::move(rhs).m_call, (View&&)vw);
        }

    };

#define ACTION_GENERATOR(object)                                                \
    [[maybe_unused]]                                                            \
    constexpr auto object = []<typename... Args>(Args&&... args)                \
    {                                                                           \
        auto func = bind_back(std::ranges:: object , std::forward<Args>(args)...);   \
        return action_wrapper<std::remove_cvref_t<decltype(func)>>(std::move(func));                  \
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

