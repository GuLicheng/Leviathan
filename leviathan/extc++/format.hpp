#pragma once

#include <format>
#include <variant>
#include <meta>
#include <optional>
#include <leviathan/annotations.hpp>
#include <leviathan/type_caster.hpp>

namespace cpp
{

// template <typename... Ts, typename CharT>
// struct default_variant_formatter
// {
//     template <typename ParseContext>
//     constexpr typename ParseContext::iterator parse(ParseContext& ctx)
//     {
//         return ctx.begin();
//     }

//     template <typename FormatContext>
//     typename FormatContext::iterator format(const std::variant<Ts...>& v, FormatContext& ctx) const
//     {
//         return std::visit(
//             [&](const auto& value) { 
//                 return std::format_to(ctx.out(), "{}", value);
//             }, v
//         );
//     }
// };

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

} // namespace cpp

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

template <typename Enum>
struct universal_enum_formatter
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
template <> struct std::formatter<Gender> : cpp::universal_enum_formatter<Gender> { };
*/



// TODO: add annotation version


}  // namespace cpp
