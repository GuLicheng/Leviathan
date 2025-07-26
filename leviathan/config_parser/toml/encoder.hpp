#pragma once

#include <leviathan/config_parser/toml/value.hpp>
#include <leviathan/config_parser/formatter.hpp>
#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/concepts.hpp>
#include <ranges>
#include <utility>

namespace cpp::config::toml
{
    
struct encoder
{
    static std::string operator()(const integer& x)
    {
        return std::format("{}", x);
    }

    static std::string operator()(const floating& x)
    {
        return std::format("{}", x);
    }

    static std::string operator()(const string& x)
    {
        return std::format("\"{}\"", x);
    }

    static std::string operator()(const datetime& x)
    {
        return x.to_string();
    }

    static std::string operator()(const boolean& x)
    {
        return x ? "true" : "false";
    }

    static std::string operator()(const array& arr)
    {
        return format_sequence(encoder(), arr, ", ");
    }

    static std::string operator()(const table& tbl)
    {
        return format_map(encoder(), tbl, "{} = {}", ", ");
    }

    static std::string operator()(const value& v)
    {
        return std::visit([]<typename T>(const T& x) {
            return encoder::operator()(value::accessor()(x));
        }, v.data());
    }
};

namespace detail
{

template <typename T>
struct caster;

template <>
struct caster<std::string>
{
    static std::string operator()(const value& v)
    {
        return encoder()(v);
    }
};

template <cpp::meta::arithmetic Arithmetic>
struct caster<Arithmetic>
{
    static Arithmetic operator()(const value& v)
    {
        if (v.is<integer>())
        {
            return static_cast<Arithmetic>(v.as<integer>());
        }
        else if (v.is<floating>())
        {
            return static_cast<Arithmetic>(v.as<floating>());
        }
        else if (v.is<boolean>())
        {
            return v.as<boolean>() ? Arithmetic(1) : Arithmetic(0);
        }
        else if (v.is<string>())
        {
            std::string_view ctx = v.as<string>();
            auto result = type_caster<Arithmetic, std::string_view, cpp::error_policy::optional>()(ctx);

            if (result)
            {
                return *result;
            }
            else
            {
                throw std::runtime_error("Failed to convert string to number");
            }
        }
        else
        {
           throw std::runtime_error("Value is not a number");
        }
    }
};

template <std::ranges::range Container>
struct caster<Container>
{
    static Container operator()(const value& v)
    {
        using ValueType = typename Container::value_type;

        if constexpr (cpp::meta::pair_like<ValueType>)
        {
            // object
            using KeyType = std::tuple_element_t<0, ValueType>;
            using MappedType = std::tuple_element_t<1, ValueType>;

            if (v.is<table>())
            {
                return v.as<table>() | std::views::transform([](const auto& pair) {
                    return std::pair<KeyType, MappedType>(pair.first, caster<MappedType>()(pair.second));
                }) | std::ranges::to<Container>();
            }
            else
            {
                throw std::runtime_error("Value is not an object");
            }
        }
        else
        {
            // array
            if (v.is<array>())
            {
                return v.as<array>() | std::views::transform(caster<ValueType>()) | std::ranges::to<Container>();
            }
            else
            {
                throw std::runtime_error("Value is not an array");
            }
        }
    }
};


}  // namespace detail

} // namespace cpp::config::toml

template <typename Target>
struct cpp::type_caster<Target, cpp::toml::value, cpp::error_policy::exception>
{
    static auto operator()(const cpp::toml::value& v)
    {
        return cpp::toml::detail::caster<Target>()(v);
    }
};
