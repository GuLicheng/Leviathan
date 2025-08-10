#pragma once

#include <leviathan/config_parser/json/value.hpp>
#include <leviathan/extc++/concepts.hpp>
#include <leviathan/extc++/functional.hpp>
#include <leviathan/type_caster.hpp>

#include <cassert>

namespace cpp::config::json
{
    
namespace detail
{

struct encoder
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
       auto context = arr 
                    | cpp::views::transform_join_with(encoder(), ',')
                    | std::ranges::to<std::string>();
        return std::format("[{}]", context);
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
        auto kv2string = [=]<typename PairLike>(PairLike&& kv) 
        {
            return std::format("{}:{}",
                encoder()(std::get<0>((PairLike&&)kv)), 
                encoder()(std::get<1>((PairLike&&)kv))
            );
        };

        auto context = obj
                     | cpp::views::transform_join_with(kv2string, ',')
                     | std::ranges::to<std::string>();
        return std::format("{{{}}}", context);
    }

    [[noreturn]] static std::string operator()(const error_code& ec)
    {
        std::unreachable();
    }

    static std::string operator()(const value& v) 
    {
        return std::visit([]<typename T>(const T& x) {
            return encoder::operator()(value::accessor()(x));
        }, v.data());
    }
};

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
        if (v.is<number>())
        {
            return v.as<number>().as<Arithmetic>();
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

            if (v.is<object>())
            {
                return v.as<object>()
                     | cpp::views::pair_transform(std::identity(), caster<MappedType>())
                     | std::ranges::to<Container>();
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
                return v.as<array>() 
                     | std::views::transform(caster<ValueType>()) 
                     | std::ranges::to<Container>();
            }
            else
            {
                throw std::runtime_error("Value is not an array");
            }
        }
    }
};

/*
{
    "name": "Alice",
    "age": 30,
    "is_student": false,
    "grades": [
        85,
        90,
        78
    ],
    "address": {
        "street": "123 Main St",
        "city": "Wonderland",
        "zip": "12345"
    }
} 
*/
struct indented_encoder
{

    indented_encoder(int count) : m_count(count) {}

    std::string m_result;
    int m_level = 0;
    int m_count;

    std::string indent() const
    {
        return std::string(m_count * m_level, ' ');
    }

    void operator()(const number& number)
    {
        m_result += std::visit(cpp::to_string, number.data());
    }

    void operator()(const string& str) 
    {
        m_result += std::format("\"{}\"", str);
    }

    void operator()(const array& arr) 
    {
        m_result += "[\n";
        m_level++;

        for (std::size_t i = 0; i < arr.size(); ++i)
        {
            m_result += indent();
            this->operator()(arr[i]);

            if (i != arr.size() - 1) 
            {
                m_result.append(",\n");
            }
        }

        m_result += "\n";
        m_level--;
        m_result += indent() + "]";
    }

    void operator()(const boolean& boolean) 
    {
        m_result.append((boolean ? "true" : "false"));
    }

    void operator()(const null&) 
    {
        m_result.append("null"); 
    }

    void operator()(const object& object) 
    {
        m_result += "{\n";

        auto size = object.size();
        auto idx = 0;
        m_level++;

        for (auto it = object.begin(); it != object.end(); ++it, idx++)
        {
            m_result += indent() + std::format(R"("{}" : )", it->first);

            this->operator()(it->second);

            if (idx != size - 1) 
            {
                m_result += ",\n";
            }
        }
        
        m_result += "\n";
        m_level--;
        m_result += indent() + "}";
    }

    void operator()(const error_code& ec)
    {
        std::unreachable();
    }

    void operator()(const value& value) 
    {
        std::visit([this]<typename T>(const T& x) {
            this->operator()(value::accessor()(x));
        }, value.data());
    }
};

}  // namespace detail

inline std::string dumps(const value& x, int indent = 0)
{
    using NoneEncoder = detail::encoder;
    using IndentedEncoder = detail::indented_encoder;

    if (indent == 0)
    {
        return NoneEncoder()(x);
    }
    else
    {
        IndentedEncoder encoder(indent);
        encoder(x);
        return std::move(encoder.m_result);
    }
}

inline void dump(const value& x, const char* filename, int indent = 0) 
{
    auto context = dumps(x, indent);
    write_file(context, filename);
}

} // namespace cpp::config::json

// https://blog.csdn.net/jkddf9h8xd9j646x798t/article/details/127954236
// template <typename CharT>
template <>
struct std::formatter<cpp::json::value, char> 
{
    // enum format_kind { indented, none };

    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        auto symbol = std::ranges::find(ctx.begin(), ctx.end(), '}');
        std::string_view fmt = std::string_view(ctx.begin(), symbol);

        if (fmt.empty())
        {
            m_indent = 0; // default no indent
        }
        else if (fmt[0] == 'i' || fmt[0] == 'I')
        {
            m_indent = cpp::cast<int>(fmt.substr(1));
        }
        else    
        {
            throw std::format_error("Invalid format specifier for json::value");
        }

        assert(m_indent >= 0 && m_indent <= 8 && "Indentation level must be between 0 and 8");
        return symbol; // return the end iterator
    }

    template <typename FmtContext>
    typename FmtContext::iterator format(const cpp::json::value& value, FmtContext& ctx) const
    {
        auto result = cpp::json::dumps(value, m_indent);        
        return std::ranges::copy(result, ctx.out()).out;
    }   

private:

    int m_indent;
};

template <typename Target>
struct cpp::type_caster<Target, cpp::json::value, cpp::error_policy::exception>
{
    static auto operator()(const cpp::json::value& v)
    {
        return cpp::json::detail::caster<Target>()(v);
    }
};
