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
    constexpr auto indices = cpp::refl::indices_without_removed_member<T, cpp::refl::skip>(); 
    return indices.size();
}
    
template <typename T, size_t N>
consteval std::meta::info tuple_element_type()
{
    constexpr auto ctx = std::meta::access_context::unchecked();
    constexpr auto members = define_static_array(nonstatic_data_members_of(^^T, ctx));
    constexpr auto [...indices] = cpp::refl::indices_without_removed_member<T, cpp::refl::skip>(); 
    return members[indices...[N]];
}

template <typename... Ts>
consteval std::meta::info define_basic_tuple()
{
    struct implementation;

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

        define_aggregate(^^implementation, member_specs);
    }

    return ^^implementation;
}

template <typename... Ts>
using basic_tuple = typename [:define_basic_tuple<Ts...>():];

}  // namespace detail
  
/**
 * @brief A tuple-like interface for class type. The class should be annotated with [[=cpp::derive::tuple_like]],
 *      and its members that should be treated as tuple elements should be annotated with [[=cpp::refl::tuple_element]].
 *      Since C++ use ADL to find the get function, the global or std namespace is not a good place to put the get function, 
 *      we can put it in a base class, and let the tuple-like class inherit from it.
 * 
 * @example
 *   class [[=cpp::derive::tuple_like]] Point : public cpp::tuple_get_interface
 *   {
 *      int X;
 *      double Y;
 * 
 *   public:
 *      Point(int x, double y) : X(x), Y(y) {}
 *   };
 * 
 *  static_assert(std::tuple_size<Point>::value == 2);
 *  static_assert(std::is_same_v<std::tuple_element<0, Point>::type, int>);
 *  static_assert(std::is_same_v<std::tuple_element<1, Point>::type, double>);
 *  Point p{1, 3.14};
 *  auto [x, y] = p; // structured binding
 *  assert(x == 1 && y == 3.14);
 */
struct tuple_get_interface
{
    template <size_t N, typename Self>
    constexpr decltype(auto) get(this Self&& tl)
    {
        using DTuple = std::remove_cvref_t<Self>;
        constexpr auto member = cpp::detail::tuple_element_type<DTuple, N>();
        return std::forward_like<Self>(tl.[:member:]);
    }
};

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

template <typename T>
    requires (std::is_class_v<T> && cpp::refl::has_annotation(^^T, cpp::derive::tuple_like))
struct std::tuple_size<T> : std::integral_constant<size_t, cpp::detail::count_tuple_elements<T>()> { };

template <size_t N, typename T>
    requires (std::is_class_v<T> && cpp::refl::has_annotation(^^T, cpp::derive::tuple_like))
struct std::tuple_element<N, T> : std::type_identity<typename [:type_of(cpp::detail::tuple_element_type<T, N>()):]> { };


