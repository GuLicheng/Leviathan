#pragma once

#include <leviathan/config_parser/toml/value.hpp>
#include <leviathan/config_parser/json/value.hpp>
#include <leviathan/type_caster.hpp>

#include <algorithm>
#include <ranges>

namespace cpp::config
{

struct toml2json
{
    static json::value operator()(const toml::value& tv) 
    {
        return std::visit([]<typename T>(const T& x) {
            return toml2json::operator()(toml::value::accessor()(x));
        }, tv.data());
    }

    static json::value operator()(const toml::integer& x)
    {
        return json::make_json<json::number>(x);
    } 

    static json::value operator()(const toml::boolean& x)
    {
        return json::make_json<json::boolean>(x);
    }

    static json::value operator()(const toml::floating& x)
    {
        return json::make_json<json::number>(x);
    }

    static json::value operator()(const toml::string& x)
    {
        return json::make_json<json::string>(x);
    }

    static json::value operator()(const toml::array& x)
    {
        auto retval = x | std::views::transform(toml2json()) | std::ranges::to<json::array>();
        return json::make_json<json::array>(std::move(retval));
    }

    static json::value operator()(const toml::table& x)
    {
        auto table2object = cpp::views::pair_transform(
            cpp::cast<json::string>,
            toml2json()
        );
        return json::make_json<json::object>(
            x | table2object | std::ranges::to<json::object>()
        );
    }

    static json::value operator()(const toml::datetime& x)
    {
        return json::make_json<json::string>(
            x.to_string()
        );
    }
}; 

struct json2toml
{
    static toml::value operator()(const json::boolean& x)
    {
        return toml::make_toml<toml::boolean>(x);
    }

    static toml::value operator()(const json::null& x)
    {
        return toml::make_toml<toml::string>("null");
    }

    static toml::value operator()(const json::number& x)
    {
        if (x.is_floating())
        {
            return toml::make_toml<toml::floating>(x.as_floating());
        }
        return x.is_signed_integer()
             ? toml::make_toml<toml::integer>(x.as_signed_integer())
             : toml::make_toml<toml::integer>(x.as_unsigned_integer());
    }

    static toml::value operator()(const json::string& x)
    {
        return toml::make_toml<toml::string>(x);
    }

    static toml::value operator()(const json::array& x)
    {
        return toml::make_toml<toml::array>(
            x | std::views::transform(json2toml()) | std::ranges::to<toml::array>(false)
        );
    }

    static toml::value operator()(const json::object& x)
    {
        auto object2table = cpp::views::pair_transform(
            cpp::cast<toml::string>,
            json2toml()
        );
        return toml::make_toml<toml::table>(
            x | object2table | std::ranges::to<toml::table>(false)
        );
    }

    static toml::value operator()(const json::value& jv)
    {
        return std::visit([]<typename T>(const T& x) {
            return json2toml::operator()(json::value::accessor()(x));
        }, jv.data());
    }
};

} // namespace cpp::config

template <>
struct cpp::type_caster<cpp::json::value, cpp::toml::value, cpp::error_policy::exception>
{
    using result_type = cpp::json::value;

    static auto operator()(const cpp::toml::value& v)
    {
        return cpp::config::toml2json()(v);
    }
};

template <>
struct cpp::type_caster<cpp::toml::value, cpp::json::value, cpp::error_policy::exception>
{
    using result_type = cpp::toml::value;

    static auto operator()(const cpp::json::value& v)
    {
        return cpp::config::json2toml()(v);
    }
};