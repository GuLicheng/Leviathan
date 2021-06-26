#pragma once

#include <ranges>
#include <functional>
#include <lv_cpp/io/console.hpp>



namespace leviathan::views
{

    template <typename Container, typename... Args>
    struct to_fn
    {
        std::tuple<Args...> m_args;
        constexpr static size_t size = std::tuple_size_v<decltype(m_args)>;

        constexpr to_fn(Args... args) : m_args{args...} 
        {
            console::write_line(m_args);
        }

        template <typename Range>
        constexpr auto operator()(Range&& range)
        {   
            return this->get_params(std::forward<Range>(range), m_args, 
                std::make_index_sequence<size>());
        }

        template <typename Range, auto... Idx>
        auto get_params(Range&& range, std::tuple<Args...>& t, std::index_sequence<Idx...>)
        {
            if constexpr (size == 0)
                return Container(range.begin(), range.end());
            else
                return Container(range.begin(), range.end(), std::get<Idx>(t) ...);
        }

        template <typename Range>
        constexpr friend auto operator|(Range range, to_fn rhs)
        {
            return rhs(std::forward<Range>(range));
        }

    };

    template <typename Container, typename... Args>
    constexpr auto to(Args&&... args)
    {
        // use args... to initialize Container
        return to_fn<Container, Args...>(std::forward<Args>(args)...);
    }

}