#pragma once

#include <format>
#include <variant>
#include <meta>
#include <optional>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/type_caster.hpp>

namespace cpp
{

struct universal_formatter 
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) 
    { 
        // auto symbol = std::ranges::find(ctx.begin(), ctx.end(), '}');
        // std::string_view fmt = std::string_view(ctx.begin(), symbol);
        // return symbol; // return the end iterator
        return ctx.begin();
    }

    template <typename T, typename FmtContext>
    typename FmtContext::iterator format(const T& t, FmtContext& ctx) const
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
            delim();
            std::string mem_label = has_identifier(mem) ? refl::extract_name_by_annotation<mem, ^^T>()
                : "(unnamed-member)";

            out = std::format_to(out, "{}: {}", mem_label, t.[:mem:]);
        }

        *out++ = '}';
        return out;
    }

};

struct enum_formatter
{
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template <typename EnumType>
    static auto format(EnumType value, auto& ctx) 
    {
        return std::format_to(ctx.out(), "{}", enum_encoder<EnumType>()(value));
    }
};

} // namespace cpp

template <typename T, typename CharT>
    requires (cpp::refl::has_annotation(^^T, cpp::derive::debug) && std::is_class_v<T>)
struct std::formatter<T, CharT> : cpp::universal_formatter { };

template <typename T, typename CharT>
    requires (cpp::refl::has_annotation(^^T, cpp::derive::debug) && std::is_enum_v<T>)
struct std::formatter<T, CharT> : cpp::enum_formatter { };
