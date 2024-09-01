#pragma once

#include "../formatter.hpp"
#include "value.hpp"

namespace leviathan::config::json
{
    
struct encoder
{
    std::string m_result;

    void operator()(const number& number, int count)
    {
        m_result += std::visit(
            [](const auto& x) { return std::format("{}", x); },
            number.data()
        );
    }

    void operator()(const string& str, int count) 
    {
        m_result += std::format("\"{}\"", str);
    }

    void operator()(const array& arr, int count) 
    {
        m_result += '[';

        for (std::size_t i = 0; i < arr.size(); ++i)
        {
            if (i != 0) 
            {
                m_result.append(", ");
            }
            this->operator()(arr[i], 0);
        }

        m_result += ']';
    }

    void operator()(const boolean& boolean, int count) 
    {
        m_result.append((boolean ? "true" : "false"));
    }

    void operator()(const null&, int count) 
    {
        m_result.append("null"); 
    }

    void operator()(const object& object, int count) 
    {
        m_result += '{';
        for (auto it = object.begin(); it != object.end(); ++it)
        {
            if (it != object.begin()) 
            {
                m_result += ", ";
            }
            m_result += std::format(R"("{}" : )", it->first);
            this->operator()(it->second, count + 1);
        }
        m_result += '}';
    }

    void operator()(const error_code& ec, int count)
    {
        std::unreachable();
    }

    void operator()(const value& value, int count) 
    {
        std::visit([count, this]<typename T>(const T& x) {
            this->operator()(value::accessor()(x), count);
        }, value.data());
    }
};

struct encoder2
{
    static std::string operator()(const number& num)
    {
        return std::visit(
            [](const auto& x) { return std::format("{}", x); },
            num.data()
        );
    }

    static std::string operator()(const string& str) 
    {
        return std::format("\"{}\"", str);
    }

    static std::string operator()(const array& arr) 
    {
        return format_sequence(encoder2(), arr, ",", "[{}]");
    }

    static std::string operator()(const boolean& boolean) 
    {
        return boolean ? "true" : "false";
    }

    static std::string operator()(const null&) 
    {
        return "null";
    }

    static std::string operator()(const object& obj) 
    {
        return format_map(encoder2(), obj);
    }

    [[noreturn]] static std::string operator()(const error_code& ec)
    {
        std::unreachable();
    }

    static std::string operator()(const value& v) 
    {
        return std::visit([]<typename T>(const T& x) {
            return encoder2::operator()(value::accessor()(x));
        }, v.data());
    }
};

inline std::string dumps(const value& x)
{
    encoder e;
    e(x, 0);
    return e.m_result;
}

inline void dump(const value& x, const char* filename) 
{
    auto context = dumps(x);
    write_file(context, filename);
}

} // namespace leviathan::config::json

template <typename CharT>
struct std::formatter<leviathan::json::value, CharT> 
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FmtContext>
    typename FmtContext::iterator format(const leviathan::json::value& value, FmtContext& ctx) const
    {
        // auto result = leviathan::json::dumps(value);
        auto result = leviathan::json::encoder2()(value);
        return std::ranges::copy(result, ctx.out()).out;
    }   
};
