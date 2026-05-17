#pragma once

#include <leviathan/annotations/all.hpp>
#include <meta>

namespace cpp
{

template <typename Enum>
struct enum_decoder
{
    static_assert(std::is_enum_v<Enum>, "universal_enum_parser can only be used with enum types");

    static constexpr Enum operator()(std::string_view str)
    {
        template for (constexpr auto e : define_static_array(enumerators_of(^^Enum)))
            if (str == refl::extract_name_by_annotation<e, ^^Enum>())
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
                return refl::extract_name_by_annotation<e, ^^Enum>();
        throw std::runtime_error(std::format("Invalid enum value: {}", static_cast<std::underlying_type_t<Enum>>(value)));
    }
};

}
 
template <typename Enum>
    requires (std::is_enum_v<Enum> && cpp::refl::has_annotation(^^Enum, cpp::derive::op_pipe))
constexpr Enum operator|(Enum x, Enum y)
{
    return Enum(std::to_underlying(x) | std::to_underlying(y));
}

template <typename Enum>
    requires (std::is_enum_v<Enum> && cpp::refl::has_annotation(^^Enum, cpp::derive::op_pipe))
constexpr Enum& operator|=(Enum& x, Enum y)
{
    x = Enum(std::to_underlying(x) | std::to_underlying(y));
    return x;
}

