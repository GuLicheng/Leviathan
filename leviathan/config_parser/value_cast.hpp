#pragma once

#include "json/value.hpp"
#include "toml/value.hpp"

#include <algorithm>
#include <ranges>

namespace leviathan::config
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
        auto table2object = [](const auto& kv) static 
        {
            return std::make_pair(
                json::string(kv.first), 
                toml2json::operator()(kv.second));
        };

        auto retval = x 
                    | std::views::transform(table2object)
                    | std::ranges::to<json::object>();
        return json::make_json<json::object>(std::move(retval));
    }

    static json::value operator()(const toml::datetime& x)
    {
        return json::string(x.to_string());
    }
}; 

struct json2toml
{
    static toml::value operator()(const json::boolean& x)
    {
        return x;
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
        auto retval = x | std::views::transform(json2toml()) | std::ranges::to<toml::array>(false);
        return toml::make_toml<toml::array>(std::move(retval));
    }

    static toml::value operator()(const json::object& x)
    {
        auto object2table = [](const auto& kv) static 
        {
            return std::make_pair(
                toml::string(kv.first), 
                json2toml::operator()(kv.second));
        };

        auto retval = x 
                    | std::views::transform(object2table)
                    | std::ranges::to<toml::table>(false);
        return toml::make_toml<toml::table>(std::move(retval));
    }

    static toml::value operator()(const json::value& jv)
    {
        return std::visit([]<typename T>(const T& x) {
            return json2toml::operator()(json::value::accessor()(x));
        }, jv.data());
    }

    [[noreturn]] static toml::value operator()(const json::error_code& x)
    {
        std::unreachable();
    }
};

} // namespace leviathan::config