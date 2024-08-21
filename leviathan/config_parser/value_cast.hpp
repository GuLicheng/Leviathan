#pragma once

#include "json/value.hpp"
#include "toml/value.hpp"

namespace leviathan::config
{
    
template <typename Target, typename Source> struct value_cast;

} // namespace leviathan::config

namespace leviathan::config
{

namespace detail
{

struct toml2json
{
    static json::value operator()(const toml::value& value) 
    {
        return std::visit([]<typename T>(const T& x) {
            return toml2json::operator()(toml::value::accessor()(x));
        }, value.data());
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
        auto retval = x 
                    | std::views::transform([=](const auto& kv) /* -> std::pair<json::string, json::value> */ {
                        return std::make_pair(
                            toml2json::convert_string(kv.first),
                            toml2json::operator()(kv.second)); })
                    | std::ranges::to<json::object>();
        return json::make_json<json::object>(std::move(retval));
    }

    static json::value operator()(const toml::datetime& x)
    {
        return json::string(x.to_string());
    }

private:

    static toml::string convert_string(const toml::string& x) 
    {
        if constexpr (std::is_same_v<json::string, toml::string>)
        {
            return x;
        }
        else
        {
            return toml2json::operator()(x).template as<json::string>();
        }
    };
}; 

}

template <> 
struct value_cast<json::value, toml::value> : detail::toml2json
{ };

} // namespace leviathan::config