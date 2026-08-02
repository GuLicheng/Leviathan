#pragma once

#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/annotation.hpp>
#include <leviathan/type_caster.hpp>
#include <format>

namespace cpp
{

template <typename Enum, bool UseUnderlying, bool Exception>
struct default_enum_decoder;

template <typename Enum, bool UseUnderlying, bool Exception>
struct default_enum_decoder
{
    static constexpr std::optional<Enum> operator()(std::string_view str) requires (!UseUnderlying)
    {
        template for (constexpr auto e : define_static_array(enumerators_of(^^Enum)))
            if (str == refl::handle<e>::identifier())
                return [:e:];
        return std::nullopt;
    }

    static constexpr std::optional<Enum> operator()(std::underlying_type_t<Enum> x) 
    {
        template for (constexpr auto e : define_static_array(enumerators_of(^^Enum)))
            if (x == std::to_underlying([:e:]))
                return [:e:];
        return std::nullopt;
    }
};

template <typename Enum, bool UseUnderlying>
struct default_enum_decoder<Enum, UseUnderlying, true>
{
    static constexpr auto operator()(std::string_view x)
    {
        auto result = default_enum_decoder<Enum, UseUnderlying, false>()(x);
        return result ? *result : throw std::runtime_error(std::format("Invalid enum string: {}", x));
    }

    static constexpr auto operator()(std::underlying_type_t<Enum> x)
    {
        auto result = default_enum_decoder<Enum, UseUnderlying, false>()(x);
        return result ? *result : throw std::runtime_error(std::format("Invalid enum value: {}", x));
    }
};

template <typename Enum>
using enum_int_decoder = default_enum_decoder<Enum, true, true>;

template <typename Enum>
using enum_str_decoder = default_enum_decoder<Enum, false, true>;

}
 
template <typename Enum>
    requires (std::is_enum_v<Enum> && cpp::refl::has_annotations(^^Enum, cpp::derive::op_pipe))
constexpr Enum operator|(Enum x, Enum y)
{
    return Enum(std::to_underlying(x) | std::to_underlying(y));
}

template <typename Enum>
    requires (std::is_enum_v<Enum> && cpp::refl::has_annotations(^^Enum, cpp::derive::op_pipe))
constexpr Enum& operator|=(Enum& x, Enum y)
{
    x = Enum(std::to_underlying(x) | std::to_underlying(y));
    return x;
}

