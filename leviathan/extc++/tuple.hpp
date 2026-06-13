#pragma once

#include <functional>
#include <ranges>
#include <leviathan/extc++/meta.hpp>

namespace cpp
{

namespace detail
{

template <typename T>
consteval size_t tuple_size() 
{
    static_assert(std::is_class_v<T>);
    constexpr auto indices = cpp::refl::indices_without_removed_member<T, cpp::refl::skip>();
    return indices.size();
}

template <typename T, size_t Index>
consteval std::meta::info tuple_element()
{
    static_assert(std::is_class_v<T>);
    constexpr auto indices = cpp::refl::indices_without_removed_member<T, cpp::refl::skip>();
    constexpr auto ctx = std::meta::access_context::current();
    return nonstatic_data_members_of(^^T, ctx)[std::get<Index>(indices)];
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
    // constexpr auto ctx = std::meta::access_context::current();
    // constexpr auto members = define_static_array(nonstatic_data_members_of(^^std::remove_cvref_t<TupleLike>, ctx));
    // constexpr auto size = members.size();
    // constexpr auto [...indices] = std::make_index_sequence<size>();
    // return std::invoke((Fn&&)fn, ((typename [:type_of(members[indices]):]&&)(tl.[:members[indices]:]))...);
}

template <typename... Args>
constexpr auto make_tuple(Args&&... args)
{
    return tuple<std::unwrap_ref_decay_t<Args>...>((Args&&)args...);
}

} // namespace cpp


// Extend std::tuple_size and std::tuple_element and std::get
namespace std
{

// template <typename T>
//     requires (is_class_v<T> && cpp::refl::has_annotation(^^T, cpp::derive::tuple_like))
// struct tuple_size<T> : integral_constant<size_t, cpp::detail::tuple_size<T>()> { };

// template <size_t Index, typename T>
//     requires (is_class_v<T> && cpp::refl::has_annotation(^^T, cpp::derive::tuple_like))
// struct tuple_element<Index, T>
// {
//     using type = typename [:type_of(cpp::detail::tuple_element<T, Index>()):];
// };

// template <size_t Index, typename TupleLike>
//     requires (is_class_v<remove_cvref_t<TupleLike>> && cpp::refl::has_annotation(^^TupleLike, cpp::derive::tuple_like))
// constexpr auto&& get(TupleLike&& tl) noexcept
// {
//     using DTupleLike = remove_cvref_t<TupleLike>;
//     constexpr auto indices = cpp::refl::indices_without_removed_member<DTupleLike, cpp::refl::skip>();
//     return std::forward_like<TupleLike>(tl.[:cpp::refl::member_number<DTupleLike>(std::get<Index>(indices)):]);
// }

}  // namespace std