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
           throw std::runtime_error(std::format("Value is not {} a number.", v.type_name()));
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

namespace cpp::config::toml
{

inline std::string as_std_string(const string& s)
{
    return { s.begin(), s.end() };
}

namespace detail
{

inline constexpr const char* kvfmt = "\"{}\" = {}\n";

inline constexpr auto map_to_str = [](auto&& r, auto&& f)
{
    return r | std::views::transform(f) | std::views::join | std::ranges::to<std::string>();
};

inline constexpr auto join_with = [](auto&& r, char delim)
{
    return r | std::views::join_with(delim) | std::ranges::to<std::string>();
};

inline constexpr auto format_value = []<typename Block>(Block block)
{
    auto& res = block.front();
    auto& path = res.m_path;
    
    if (path.size() == 1)
    {
        auto encode_fn = [=](const auto& v) 
        { 
            const auto& [k, vptr, _] = v;
            return std::format(kvfmt, k.front(), encoder()(*vptr)); 
        };

        return map_to_str(block, encode_fn) + '\n';
    }
    else
    {
        std::string tail = as_std_string(path.back());
        path.pop_back();

        auto encode_fn = [=](const auto& v) 
        { 
            const auto& [k, vptr, _] = v;
            return std::format(kvfmt, tail, encoder()(*vptr)); 
        };

        return std::format("[{}]\n{}\n", join_with(path, '.'), map_to_str(block, encode_fn));
    }
};

inline constexpr auto format_table_array = []<typename Block>(Block block)
{
    std::string retval = "";

    for (auto& res : block)
    {
        std::string table_name = join_with(res.m_path, '.');

        for (const auto& tbl : res.m_value_ptr->template as<toml::array>())
        {
            retval += std::format("[[{}]]\n", table_name);

            for (const auto& [k, v] : tbl.template as<toml::table>())
            {
                retval += std::format(kvfmt, k, toml::encoder()(v));
            }

            retval += '\n';
        }
    }

    return retval;
};

}  // namespace detail

struct formatter_helper
{
    enum struct leaf_kind
    {
        value, table_array
    };

    struct collect_result
    {   
        std::vector<std::string> m_path;
        const toml::value* m_value_ptr;
        leaf_kind m_kind;
    };

    struct path_collector
    {
        std::vector<collect_result> m_paths;
        
        void collect_leaf(const value& tv, std::vector<std::string>& cur_path)
        {
            if (tv.index() < 6)
            {
                m_paths.emplace_back(cur_path, &tv);
                return;
            }    

            for (const auto& [k, v] : tv.as<toml::table>())
            {
                cur_path.push_back(as_std_string(k));
                collect_leaf(v, cur_path);
                cur_path.pop_back();
            }
        }

        void collect_leaf_and_table_array(const value& tv, std::vector<std::string>& cur_path)
        {
            if (tv.index() < 5)
            {
                m_paths.emplace_back(cur_path, &tv, leaf_kind::value);
                return;
            }    

            if (tv.is<toml::table>())
            {
                for (const auto& [k, v] : tv.as<toml::table>())
                {
                    cur_path.push_back(as_std_string(k));
                    collect_leaf_and_table_array(v, cur_path);
                    cur_path.pop_back();
                }
            }
            else
            {
                bool is_table_array = std::ranges::all_of(
                    tv.as<toml::array>(), 
                    [](const auto& v) { return v.template is<toml::table>(); }
                );
    
                m_paths.emplace_back(cur_path, &tv, (is_table_array ? leaf_kind::table_array : leaf_kind::value));
            }
        }

        std::vector<collect_result> operator()(const toml::value& tv, bool stop_for_array_table)
        {
            m_paths.clear();
            std::vector<std::string> v;
            stop_for_array_table ? collect_leaf_and_table_array(tv, v) : collect_leaf(tv, v);
            return std::move(m_paths);
        }
    };

    static std::string operator()(const value& tv)
    {
        auto path = path_collector()(tv, true);

        auto cmp = [](const auto& a, const auto& b) 
        {
            if (a.m_path.size() != b.m_path.size())
            {
                return a.m_path.size() < b.m_path.size();
            }
            
            if (a.m_kind != b.m_kind)
            {
                return a.m_kind < b.m_kind;
            }

            return a.m_path < b.m_path; 
        };

        auto chunk_fn = [](const auto& a, const auto& b) 
        {
            return a.m_kind == b.m_kind &&
                   a.m_path.size() == b.m_path.size() &&
                   std::ranges::equal(a.m_path.begin(), a.m_path.end() - 1, b.m_path.begin(), b.m_path.end() - 1); 
        };

        std::ranges::sort(path, cmp);

        auto format_block = []<typename Block>(Block block)
        {
            return block.front().m_kind == leaf_kind::value 
                 ? detail::format_value(block) 
                 : detail::format_table_array(block);
        };

        return path
             | std::views::chunk_by(chunk_fn)
             | std::views::transform(format_block) 
             | std::views::join 
             | std::ranges::to<std::string>();
    }
};

inline std::string dump(const value& tv)
{
    return formatter_helper()(tv);
}

}  // namespace cpp::config::toml

// Extend type_caster for cpp::config::toml::value
template <typename Target>
struct cpp::type_caster<Target, cpp::toml::value, cpp::error_policy::exception>
{
    static auto operator()(const cpp::toml::value& v)
    {
        return cpp::toml::detail::caster<Target>()(v);
    }
};

// Extend std::formatter for cpp::toml::value
template <typename CharT>
struct std::formatter<cpp::toml::value, CharT> 
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FmtContext>
    typename FmtContext::iterator format(const cpp::toml::value& value, FmtContext& ctx) const
    {
        auto result = cpp::toml::dump(value);
        return std::ranges::copy(result, ctx.out()).out;
    }   
};

