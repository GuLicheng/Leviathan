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

struct formatter
{
    static bool is_leaf(const value& x)
    {
        return x.data().index() <= 4
            || (x.is<array>() && x.as<array>().is_locked())
            || (x.is<table>() && x.as<table>().is_locked());
    }

    static std::string operator()(const value& tbl)
    {
        std::string retval;
        std::vector<std::string_view> super;
        format(tbl, super, "[{}]", retval);
        return retval;
    }

    static void format(const value& tbl, std::vector<std::string_view>& super, const char* fmt, std::string& out)
    {
        using record = std::vector<std::pair<std::string_view, const value*>>;
        record leafs, sub_tables, sub_arrays;

        for (const auto& [k, v] : tbl.as<table>())
        {
            if (is_leaf(v))
            {
                leafs.emplace_back(k, &v);
            }
            else if (v.is<table>())
            {
                sub_tables.emplace_back(k, &v);
            }
            else
            {
                sub_arrays.emplace_back(k, &v);
            }
        } 

        out += '\n';

        if (super.size())
        {
            auto keys = super | std::views::join_with('.') | std::ranges::to<std::string>();
            auto section = std::vformat(fmt, std::make_format_args(keys));
            out += section;
        }

        for (auto& leaf : leafs)
        {
            auto entry = std::format("{} = {}\n", leaf.first, encoder()(*leaf.second));
            out += entry;
        }

        for (auto& subtable : sub_tables)
        {
            super.emplace_back(subtable.first);
            format(*subtable.second, super, "[{}]\n", out);
            super.pop_back();
        }

        for (auto& subarray : sub_arrays)
        {
            for (auto& arr : subarray.second->as<array>())
            {
                super.emplace_back(subarray.first);
                format(arr, super, "[[{}]]\n", out);
                super.pop_back();
            }
        }
    }
};

inline std::string dump(const value& tv)
{
    return formatter()(tv);
}

} // namespace leviathan::config::toml

template <typename CharT>
struct std::formatter<leviathan::toml::value, CharT> 
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FmtContext>
    typename FmtContext::iterator format(const leviathan::toml::value& value, FmtContext& ctx) const
    {
        auto result = leviathan::toml::formatter()(value);
        return std::ranges::copy(result, ctx.out()).out;
    }   
};
