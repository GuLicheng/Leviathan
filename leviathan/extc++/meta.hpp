#pragma once

#include <meta>
#include <type_traits>
#include <string_view>
#include <stdexcept>
#include <string>
#include <vector>
#include <leviathan/annotations/all.hpp>

namespace cpp::refl
{
    
template <typename T>
consteval auto member_number(size_t N)
{
    auto ctx = std::meta::access_context::current();
    return std::meta::nonstatic_data_members_of(^^T, ctx)[N];
}

template <typename T>
consteval auto member_named(const char* name)
{
    auto ctx = std::meta::access_context::current();
    for (std::meta::info field : nonstatic_data_members_of(^^T, ctx))
        if (has_identifier(field) && identifier_of(field) == name)
            return field;
    throw std::runtime_error(std::format("No member named {} in type {}", name, identifier_of(^^T)));
}

// template <typename T>
// constexpr auto struct_to_tuple(T const& t) 
// {
//     constexpr auto ctx = std::meta::access_context::current();

//     constexpr std::size_t N = nonstatic_data_members_of(^^T, ctx).size();
//     constexpr auto members = define_static_array(nonstatic_data_members_of(^^T, ctx));
//     constexpr auto indices = [] {
//         std::array<int, N> indices;
//         std::ranges::iota(indices, 0);
//         return indices;
//         }();

//     constexpr auto [...Is] = indices;
//     return std::make_tuple(t.[:members[Is]:]...);
// }

} // namespace cpp::refl

