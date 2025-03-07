#pragma once

#include "value.hpp"
#include "../formatter.hpp"
#include <ranges>
#include <utility>

namespace leviathan::config::toml
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

} // namespace leviathan::config::toml


