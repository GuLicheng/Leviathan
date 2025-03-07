#pragma once

#include <leviathan/extc++/string.hpp>
#include "value.hpp"
#include <ranges>

namespace leviathan::config::toml
{

using leviathan::string::join;

struct formatter
{
    static std::string as_std_string(const toml::string& s)
    {
        return { s.begin(), s.end() };
    }

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

    template <typename Block>
    static void format_block(Block block, std::string& retval)
    {
        if (block.front().m_kind == leaf_kind::value)
        {
            format_value(block, retval);
        }
        else
        {
            format_table_array(block, retval);
        }
    }

    template <typename Block>
    static void format_value(Block block, std::string& retval)
    {
        auto& res = block.front();
        auto& path = res.m_path;
        
        if (path.size() == 1)
        {
            // Global value
            for (const auto& [k, vptr, _] : block)
            {
                retval += std::format("{} = {}\n", k.front(), toml::encoder()(*vptr));
            }
        }
        else
        {
            
            std::string tail = as_std_string(path.back());
            path.pop_back();
            retval += std::format("[{}]\n", join(path, "."));

            for (const auto& [k, vptr, _] : block)
            {
                retval += std::format("{} = {}\n", tail, toml::encoder()(*vptr));
            }
        }

        retval += "\n";
    }

    template <typename Block>
    static void format_table_array(Block block, std::string& retval)
    {
        for (auto& res : block)
        {
            std::string table_name = join(res.m_path, ".");

            for (const auto& tbl : res.m_value_ptr->template as<toml::array>())
            {
                retval += std::format("[[{}]]\n", table_name);

                for (const auto& [k, v] : tbl.template as<toml::table>())
                {
                    retval += std::format("{} = {}\n", k, toml::encoder()(v));
                }

                retval += '\n';
            }
        }
    }

    static std::string operator()(const value& tv)
    {
        auto path = path_collector()(tv, true);

        std::ranges::sort(path, [](const auto& a, const auto& b) {
            if (a.m_path.size() != b.m_path.size())
            {
                return a.m_path.size() < b.m_path.size();
            }
            
            if (a.m_kind != b.m_kind)
            {
                return a.m_kind < b.m_kind;
            }

            return a.m_path < b.m_path; 
        });

        auto blocks = std::views::chunk_by(path, [](const auto& a, const auto& b) {
            return a.m_kind == b.m_kind &&
                   a.m_path.size() == b.m_path.size() &&
                   std::ranges::equal(a.m_path.begin(), a.m_path.end() - 1, b.m_path.begin(), b.m_path.end() - 1); 
        });
        std::string retval = "\n";

        for (auto block : blocks)
        {
            format_block(block, retval);
        }
    
        return retval;
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