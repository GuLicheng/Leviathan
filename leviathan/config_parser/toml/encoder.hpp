#pragma once

#include "value.hpp"

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
        return std::format("{}", x);
    }

    static std::string operator()(const datetime& x)
    {
        throw std::runtime_error("Not implemented.");
    }

    static std::string operator()(const boolean& x)
    {
        return x ? "true" : "false";
    }

    static std::string operator()(const array& arr)
    {
    }

    static std::string operator()(const table& t)
    {
        
    }

    static std::string operator()(const value& x)
    {
        // All value in path is toml::string
        std::vector<value*> path = { &x };

        while (path.size())
        {
            auto top = path.back();
            path.pop_back();

            if (!top->is<table>() || (top->is<table>() && top->as<table>().empty()))
            {
                // Leaf
            }
        }
    }
};

} // namespace leviathan::config::toml
