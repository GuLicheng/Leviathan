#pragma once

#include <functional>
#include <ranges>
#include <meta>

namespace cpp
{

namespace detail
{

template <typename... Ts>
consteval auto define_basic_tuple()
{
    struct storage;

    struct int2string
    {
        static constexpr std::string operator()(std::size_t index) 
        {
            std::string result;

            do 
            {
                result.push_back('0' + (index % 10));
                index /= 10;
            } while (index > 0);
            result += '_';

            return std::string(result.rbegin(), result.rend());
        }
    };

    consteval
    {
        auto member_specs = std::vector<std::meta::info>{ ^^Ts... } 
                          | std::views::enumerate 
                          | std::views::transform([](auto indexed_info) {
                                auto [index, info] = indexed_info;
                                return data_member_spec(info, { .name = int2string{}(index), .no_unique_address = true }); 
                            });

        std::meta::define_aggregate(^^storage, member_specs);
    }

    return ^^storage;
}

template <typename... Ts>
using basic_tuple = typename [:define_basic_tuple<Ts...>():];

}  // namespace detail

template <typename... Ts>
struct tuple : detail::basic_tuple<Ts...>
{
    using base = detail::basic_tuple<Ts...>;

    template <typename... Args>
    constexpr tuple(Args&&... args) : base((Args&&)args...) {}

};

template <typename... Ts>
tuple(Ts&&...) -> tuple<Ts...>;

template <typename Fn, typename TupleLike>
constexpr auto apply(Fn&& fn, TupleLike&& tl)
{
    auto&& [...elements] = (TupleLike&&)tl;
    return std::invoke((Fn&&)fn, (decltype(elements)&&)elements...);
}

template <typename... Args>
constexpr auto make_tuple(Args&&... args)
{
    return tuple<std::unwrap_ref_decay_t<Args>...>((Args&&)args...);
}


} // namespace cpp

