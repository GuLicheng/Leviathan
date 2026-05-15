#pragma once

#include "concepts.hpp" 
#include <leviathan/annotations/all.hpp>

namespace cpp
{

template <typename T>
static constexpr void simple_hash(size_t& seed, const T& t) 
{
    seed ^= std::hash<T>()(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename TupleLike>
static constexpr size_t tuple_hash(const TupleLike& t) 
{
    size_t seed = 0;

    template for (const auto& element : t) 
    {
        simple_hash(seed, element);
    }

    return seed;
}

struct struct_hasher 
{
    template <typename T>
    static constexpr size_t operator()(const T& t) 
    {
        size_t seed = 0;

        constexpr auto unchecked = std::meta::access_context::unchecked();

        template for (constexpr auto base : define_static_array(bases_of(^^T, unchecked))) 
        {
            simple_hash(seed, static_cast<typename[:type_of(base):]&>(t));
        }

        template for (constexpr auto mem : define_static_array(nonstatic_data_members_of(^^T, unchecked))) 
        {
            simple_hash(seed, t.[:mem:]);
        }

        return seed;
    }
};

struct enum_hasher
{
    template <typename EnumType>
        requires (std::is_enum_v<EnumType>)
    static constexpr size_t operator()(EnumType e)
    {
        return std::hash<std::underlying_type_t<EnumType>>() (std::to_underlying(e));
    }
};

}

template <typename T>
    requires (cpp::refl::has_annotation(^^T, cpp::derive::hash) && std::is_class_v<T>)
struct std::hash<T> : cpp::struct_hasher { };

template <typename Enum>
    requires (cpp::refl::has_annotation(^^Enum, cpp::derive::hash) && std::is_enum_v<Enum>)
struct std::hash<Enum> : cpp::enum_hasher { };

