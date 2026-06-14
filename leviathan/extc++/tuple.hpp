#pragma once

#include <functional>
#include <ranges>
#include <leviathan/extc++/meta.hpp>

namespace cpp
{

namespace detail
{

template <typename T>
consteval size_t count_tuple_elements()
{
    constexpr auto ctx = std::meta::access_context::unchecked();
    size_t count = 0;
    template for (constexpr std::meta::info member : define_static_array(nonstatic_data_members_of(^^T, ctx)))
        if (has_annotation(member, cpp::refl::tuple_element))
            count++;
    return count;
}
    
template <typename... Ts>
consteval auto define_basic_tuple()
{
    struct storage;

    consteval
    {
        auto int2string = [](auto index_info) {
            auto [index, info] = index_info;
            std::string result;

            do 
            {
                result.push_back('0' + (index % 10));
                index /= 10;
            } while (index > 0);
            
            result += '_';
            
            // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3391r2.html
            // P3391 - constexpr std::format
            // return data_member_spec(info, { .name = std::format("_{}", index), .no_unique_address = true }); 
            return data_member_spec(info, { .name = std::string(result.rbegin(), result.rend()), .no_unique_address = true }); 
        };

        auto member_specs = std::vector<std::meta::info>{ ^^Ts... } 
                          | std::views::enumerate 
                          | std::views::transform(int2string);

        define_aggregate(^^storage, member_specs);
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


