#pragma once

#include <leviathan/annotations/all.hpp>
#include <meta>

namespace cpp
{

template <typename Enum, bool Exception = true>
struct enum_decoder
{
    static_assert(std::is_enum_v<Enum>, "universal_enum_parser can only be used with enum types");

    static constexpr Enum operator()(std::string_view str)
    {
        auto result = enum_decoder<Enum, !Exception>()(str);
        return result ? *result : throw std::runtime_error(std::format("Invalid enum string: {}", str));
    }

    static constexpr std::optional<Enum> operator()(std::string_view str) requires (!Exception)
    {
        template for (constexpr auto e : define_static_array(enumerators_of(^^Enum)))
            if (str == refl::extract_name_by_annotation<e, ^^Enum>())
                return [:e:];
        return std::nullopt;
    }
};

template <typename Enum, bool Exception = true>
struct enum_encoder
{
    static_assert(std::is_enum_v<Enum>, "universal_enum_parser can only be used with enum types");

    static constexpr std::string operator()(Enum value)
    {
        auto result = enum_encoder<Enum, !Exception>()(value);
        return result ? *result : throw std::runtime_error(std::format("Invalid enum value: {}", std::to_underlying(value)));
    }

    static constexpr std::optional<std::string> operator()(Enum value) requires (!Exception)
    {
        template for (constexpr auto e : define_static_array(enumerators_of(^^Enum)))
            if (value == [:e:])
                return refl::extract_name_by_annotation<e, ^^Enum>();
        return std::nullopt;
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

