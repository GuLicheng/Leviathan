#pragma once

#include <format>
#include <variant>
#include <meta>
#include <optional>
#include <leviathan/annotations.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/type_caster.hpp>

namespace cpp
{

struct universal_formatter 
{
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template <typename T>
    static auto format(T const& t, auto& ctx) 
    {
        auto out = std::format_to(ctx.out(), "{}{{", has_identifier(^^T) ? identifier_of(^^T) : "(unnamed-type)");

        auto delim = [first = true, &out]() mutable {
            if (!first) {
                *out++ = ',';
                *out++ = ' ';
            }
            first = false;
        };

        constexpr auto unchecked = std::meta::access_context::unchecked();

        template for (constexpr auto base : define_static_array(bases_of(^^T, unchecked))) 
        {
            delim();
            out = std::format_to(out, "{}", (typename[:type_of(base):] const&)(t));
        }

        template for (constexpr auto mem : define_static_array(nonstatic_data_members_of(^^T, unchecked))) 
        {
            delim();
            std::string_view mem_label = has_identifier(mem) ? identifier_of(mem)
                : "(unnamed-member)";
            out = std::format_to(out, "{}: {}", mem_label, t.[:mem:]);
        }

        *out++ = '}';
        return out;
    }
};

// template <> struct std::formatter<SomeType> : universal_formatter { };

template <typename Enum>
struct enum_formatter
{
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template <typename EnumType>
    static auto format(EnumType value, auto& ctx) 
    {
        return std::format_to(ctx.out(), "{}", enum_encoder<EnumType>()(value));
    }
};

/*
enum class Gender { Male, Female };
template <> struct std::formatter<Gender> : cpp::enum_formatter<Gender> { };
*/

} // namespace cpp
