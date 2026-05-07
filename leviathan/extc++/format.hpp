#pragma once

#include <format>
#include <variant>
#include <meta>

template <typename... Ts, typename CharT>
struct std::formatter<std::variant<Ts...>, CharT>
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(const std::variant<Ts...>& v, FormatContext& ctx) const
    {
        return std::visit(
            [&](const auto& value) { 
                return std::format_to(ctx.out(), "{}", value);
            }, v
        );
    }
};

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
            out = std::format_to(out, ".{}={}", mem_label, t.[:mem:]);
        }

        *out++ = '}';
        return out;
    }
};

}