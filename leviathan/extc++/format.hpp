#pragma once

#include <format>
#include <variant>
#include <meta>
#include <optional>
#include <leviathan/annotations/all.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/type_caster.hpp>

namespace cpp
{

struct universal_formatter 
{
    template <typename ParseContext>
    static constexpr typename ParseContext::iterator parse(ParseContext& ctx) 
    { 
        return ctx.begin(); 
    }

    template <typename T, typename FmtContext>
    static typename FmtContext::iterator format(const T& t, FmtContext& ctx) 
    {
        auto out = std::format_to(ctx.out(), "{}{{", display_string_of(^^T));

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
            if constexpr (refl::has_annotation(mem, refl::ignore))
            {
                continue;
            }

            delim();
            std::string mem_label = has_identifier(mem) ? refl::extract_name_by_annotation<mem, ^^T>()
                : "(unnamed-member)";

            out = std::format_to(out, "{}: {}", mem_label, t.[:mem:]);
        }

        *out++ = '}';
        return out;
    }
};
/*
template <> struct std::formatter<SomeType> : universal_formatter { };
*/

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
template <> struct std::formatter<Gender> : cpp::enum_formatter { };
*/

} // namespace cpp

template <typename T, typename CharT>
    requires (cpp::refl::has_annotation(^^T, cpp::derive::debug) && std::is_class_v<T>)
struct std::formatter<T, CharT> : cpp::universal_formatter { };

template <typename T, typename CharT>
    requires (cpp::refl::has_annotation(^^T, cpp::derive::debug) && std::is_enum_v<T>)
struct std::formatter<T, CharT> : cpp::enum_formatter { };
