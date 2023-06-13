// See experimental/ini.cpp

#pragma once

#include "item.hpp"

#include <leviathan/string/string_extend.hpp>
#include <leviathan/meta/template_info.hpp>

#include <algorithm>
#include <unordered_map>
#include <memory>
#include <vector>
#include <ranges>
#include <format>
#include <string>
#include <string_view>
#include <iterator>
#include <fstream>
#include <iostream>

#include <cctype>
#include <assert.h>

namespace leviathan::config::ini
{
    using std::string_view;
    using std::string;
    using leviathan::string::trim;
    using leviathan::string::ltrim;
    using leviathan::string::rtrim;

    template <typename K, typename V>
    using hashmap = std::unordered_map<
        K, 
        V, 
        leviathan::string::string_hash_keyequal,
        leviathan::string::string_hash_keyequal
    >;

    // We define some helper function here
    namespace detail
    {
        inline constexpr const char linefeed = '\n';

        inline constexpr const char* comment = ";#"; 

        inline constexpr const char eq = '=';

        inline constexpr const char lbracket = '[';     // left section

        inline constexpr const char rbracket = ']';    // right section

        inline constexpr auto get_section_or_entry_context = [](auto line) static {

            // line is std::subrange<std::string::iterator, std::string::iterator, std::ranges::subrange::sized>

            // for empty line or line just with comments, return "".

            auto str = string_view(line);

            auto end_of_line = str.find_first_of(comment);  // find comment

            auto new_line = str.substr(0, end_of_line); // remove comment

            auto line_after_trimming = trim(new_line);

            return line_after_trimming;
        };

        inline constexpr auto is_section = [](auto context) static {
            return context.front() == lbracket;
        };

        inline constexpr auto is_entry = [](auto context) static {
            return !is_section(context);
        };

        inline constexpr auto chunk_section_and_entries = [](auto l, auto r) {
            return (is_section(l) && is_entry(r)) || (is_entry(l) && is_entry(r));
        };

        inline constexpr auto split_entry_to_key_value = [](auto context) static {

            auto equal = context.find_first_of(eq);

            assert(equal != 0 && "The key should not be empty.");
            assert(equal != context.npos && "There must be a '=' in entry.");

            auto key = context.substr(0, equal);

            auto value = context.substr(equal + 1);

            return std::make_pair(rtrim(key), ltrim(value));
        };

        inline constexpr auto map_entries_to_dict = [](auto lines) static {
            hashmap<string_view, string_view> dict;
            std::ranges::for_each(lines, [&](auto line) {
                auto [k, v] = detail::split_entry_to_key_value(line);
                dict[k] = v; // If some keys appear twice at one sections, the former will be recovered.
            });
            return dict;
        };

        inline constexpr auto to_section = [](auto lines) static {
            auto section = *lines.begin();
            assert(section.front() == lbracket && section.back() == rbracket && "each section should started with '[' and end with ']'");
            auto entries = map_entries_to_dict(lines | std::views::drop(1));
            return std::make_pair(
                section.substr(1, section.size() - 2),
                std::move(entries)
            );
        };

    } // namespace detail

    class read_only_configuration
    {
        using entry_type = hashmap<string_view, string_view>;
        using section_type = hashmap<string_view, entry_type>;

        friend class configuration;

    public:

        read_only_configuration(section_type sections, std::unique_ptr<string> context)
            : m_sections(std::move(sections)), m_context(std::move(context))
        { }

        void display() const
        {
            for (const auto& [section, entries] : m_sections)
            {
                std::cout << std::format("[{}]\n", section);
                for (auto [k, v] : entries)
                    std::cout << std::format("{}={}\n", k, v);
                std::cout << '\n';
            }
        }

        item get_value(string_view section, string_view key) const
        {
            // check section
            auto it1 = m_sections.find(section);
            if (it1 == m_sections.end())
                return std::nullopt;
            
            // check key
            auto it2 = it1->second.find(key);
            if (it2 == it1->second.end())
                return std::nullopt;

            return it2->second;
        }

    private:

        section_type m_sections;
        std::unique_ptr<string> m_context; // Save file binary bytes.        
    };

    class configuration
    {
        using entry_type = hashmap<string, string>;
        using section_type = hashmap<string, entry_type>;
    
    public:

        configuration(const read_only_configuration& config)
        {
            copy_result_from_config(config);
        }

        template <typename T>
        void set_value(string section, string key, const T& value)
        {
            m_sections[std::move(section)][std::move(key)] = std::format("{}", value);
        }

        item get_value(string_view section, string_view key) const
        {
            // check section
            auto it1 = m_sections.find(section);
            if (it1 == m_sections.end())
                return std::nullopt;
            
            // check key
            auto it2 = it1->second.find(key);
            if (it2 == it1->second.end())
                return std::nullopt;

            return it2->second;
        }

        void save(std::ostream& os) const
        {
            for (const auto& [section, entries] : m_sections)
            {
                os << std::format("[{}]\n", section);
                for (auto [k, v] : entries)
                    os << std::format("{}={}\n", k, v);
                os << '\n';
            }
        }

    private:

        void copy_result_from_config(const read_only_configuration& config)
        {
            for (const auto& [section, entries] : config.m_sections)
            {
                entry_type dict;
                for (const auto& [key, value] : entries)
                {
                    dict[string(key)] = string(value);
                }
                m_sections[string(section)] = std::move(dict);
            }
        }

        section_type m_sections;
    };  

    read_only_configuration parse_configuration(const char* filename)
    {
        std::fstream ifs(filename, std::ios_base::in | std::ios_base::binary);
        
        auto context = std::make_unique<string>(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

        auto sections = *context
            | std::views::split(detail::linefeed)
            | std::views::transform(detail::get_section_or_entry_context)
            | std::views::filter(std::ranges::size)
            | std::views::chunk_by(detail::chunk_section_and_entries)  
            | std::views::transform(detail::to_section);

        hashmap<string_view, hashmap<string_view, string_view>> result;
        
        // If some entries shared same section name, we try to merge them.
        for (auto&& [section, entries] : sections)
        {
            auto [it, ok] = result.try_emplace(section, std::move(entries));
            if (!ok)
                it->second.merge(std::move(entries));
        }
        return { std::move(result), std::move(context) };
    }

} // namespace leviathan::ini

