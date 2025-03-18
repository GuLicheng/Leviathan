#pragma once

#include "value.hpp"
#include <leviathan/extc++/ranges.hpp>

namespace leviathan::config::toml
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

// inline constexpr auto format_table_array2 = []<typename Block>(Block block) 
// {
//     auto fn = []<typename T>(const T& v) 
//     {
//         std::string table_name = join_with(v.m_path, '.');

//         auto fn2 = [&](const auto& tbl) 
//         {
//             return leviathan::ranges::concat(
//                 std::format("[[{}]]\n", table_name),
//                 tbl.template as<toml::table>() | std::views::transform([](const auto& kv) { return std::format(kvfmt, kv.first, toml::encoder()(kv.second)); }) | std::views::join,
//                 std::views::single('\n')
//             ) | std::views::join;
//         };

//         return leviathan::ranges::concat(
//             table_name,
//             v.m_value_ptr->template as<toml::array>() | std::views::transform(fn2)
//         ) | std::views::join;
//     };


//     return block | std::views::transform(fn) | std::views::join | std::ranges::to<std::string>();
// };

}  // namespace detail

struct formatter
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
    return formatter()(tv);
}

}  // namespace leviathan::config::toml

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
        auto result = leviathan::toml::dump(value);
        return std::ranges::copy(result, ctx.out()).out;
    }   
};