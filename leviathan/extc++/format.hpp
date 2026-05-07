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

// template <> struct std::formatter<B> : universal_formatter { };

} // namespace cpp

namespace cpp
{

// https://isocpp.org/files/papers/P2996R13.html
// What does Enumerable mean? 
// template<typename E, bool Enumerable = is_enumerable_type(^^E)>
//     requires std::is_enum_v<E>
// constexpr std::string_view enum_to_string(E value)
// {
//     if constexpr (Enumerable)
//         template for (constexpr auto e :define_static_array(enumerators_of(^^E)))
//             if (value == [:e:])
//                 return identifier_of(e);
//     return "<unnamed>";
// }

template <typename Enum>
struct default_enum_decoder
{
    static_assert(std::is_enum_v<Enum>, "universal_enum_parser can only be used with enum types");

    static constexpr Enum operator()(std::string_view str)
    {
        template for (constexpr auto e : define_static_array(enumerators_of(^^Enum)))
            if (str == identifier_of(e))
                return [:e:];
        throw std::runtime_error("Invalid enum string");
    }
};

template <typename Enum>
struct default_enum_encoder
{
    static_assert(std::is_enum_v<Enum>, "universal_enum_parser can only be used with enum types");

    static constexpr std::string_view operator()(Enum value)
    {
        template for (constexpr auto e : define_static_array(enumerators_of(^^Enum)))
            if (value == [:e:])
                return identifier_of(e);
        return "<unnamed>";
    }
};

template <typename Enum>
struct universal_enum_formatter
{
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template <typename EnumType>
    static auto format(EnumType value, auto& ctx) 
    {
        return std::format_to(ctx.out(), "{}", default_enum_encoder<EnumType>()(value));
    }
};


// TODO: add annotation version


}  // namespace cpp
