#pragma once

#include <leviathan/annotations.hpp>

namespace cpp
{

template <typename Enum>
struct enum_decoder
{
    static_assert(std::is_enum_v<Enum>, "universal_enum_parser can only be used with enum types");

    static constexpr Enum operator()(std::string_view str)
    {
        template for (constexpr auto e : define_static_array(enumerators_of(^^Enum)))
            if (str == refl::extract_name_by_annotation<e>())
                return [:e:];
        throw std::runtime_error(std::format("Invalid enum name: {}", str));
    }
};

template <typename Enum>
struct enum_encoder
{
    static_assert(std::is_enum_v<Enum>, "universal_enum_parser can only be used with enum types");

    static constexpr std::string operator()(Enum value)
    {
        template for (constexpr auto e : define_static_array(enumerators_of(^^Enum)))
            if (value == [:e:])
                return refl::extract_name_by_annotation<e>();
        throw std::runtime_error(std::format("Invalid enum value: {}", static_cast<std::underlying_type_t<Enum>>(value)));
    }
};



}

 
