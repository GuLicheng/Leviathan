// See experimental/ini.cpp

#pragma once

#include "value.hpp"

#include <leviathan/string/string_extend.hpp>

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

namespace leviathan::ini
{
    // We define some helper function here
    namespace detail
    {
        inline constexpr const char nl = '\n';

        inline constexpr const char* comment = ";"; 

        inline constexpr const char eq = '=';

        inline constexpr const char left = '[';     // left section

        inline constexpr const char right = ']';    // right section

        inline constexpr auto trim_white_space = [](std::string_view line) {
            return leviathan::string::trim(line);
        };

        inline constexpr auto get_section_or_entry_context = [](auto line) {

            // for empty line or line just with comments, return "".

            auto str = std::string_view(line);

            auto end_of_line = str.find_first_of(comment);  // find comment

            if (end_of_line == str.npos)
                end_of_line = str.size();

            auto new_line = std::string_view(std::ranges::begin(str), end_of_line);

            auto line_after_trimming = trim_white_space(new_line);

            return line_after_trimming;
        };

        inline constexpr auto is_section = [](auto context) {
            return context.front() == '[';
        };

        inline constexpr auto is_entry = [](auto context) {
            return !is_section(context);
        };

        inline constexpr auto chunk_section_and_entries = [](auto l, auto r) {
            return (is_section(l) && is_entry(r)) || (is_entry(l) && is_entry(r));
        };

        inline constexpr auto split_entry_to_key_value = [](auto context) {

            auto equal = context.find_first_of(eq);

            assert(equal != 0 && "The key should not be empty.");
            assert(equal != context.npos && "There must be a '=' in entry.");

            auto key = context.substr(0, equal);

            auto value = context.substr(equal + 1);

            return std::make_pair(key, value);
        };

        inline constexpr auto map_entries_to_dict = [](auto lines) {
            std::unordered_map<std::string_view, std::string_view> dict;
            std::ranges::for_each(lines, [&](auto line) {
                auto [k, v] = detail::split_entry_to_key_value(line);
                dict[k] = v; // If some keys appear twice at one sections, the former will be recovered.
            });
            return dict;
        };

        inline constexpr auto to_section = [](auto lines) {
            auto section = *lines.begin();
            auto entries = map_entries_to_dict(lines | std::views::drop(1));
            return std::make_pair(
                section.substr(1, section.size() - 2),
                std::move(entries)
            );
        };

    } // namespace detail

    class read_only_configuration
    {
        using entry_type = std::unordered_map<std::string_view, std::string_view>;
        using section_type = std::unordered_map<std::string_view, entry_type>;

        friend class configuration;

    public:

        read_only_configuration(section_type sections, std::unique_ptr<std::string> context)
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

    private:

        section_type m_sections;
        std::unique_ptr<std::string> m_context; // Save file binary bytes.        
    };

    class configuration
    {
        using entry_type = std::unordered_map<std::string, std::string>;
        using section_type = std::unordered_map<std::string, entry_type>;
    
    public:

        configuration(const read_only_configuration& config)
        {
            copy_result_from_config(config);
        }

    private:

        void copy_result_from_config(const read_only_configuration& config)
        {
            for (const auto& [section, entries] : config.m_sections)
            {
                entry_type dict;
                for (const auto& [key, value] : entries)
                {
                    dict[std::string(key)] = std::string(value);
                }
                m_sections[std::string(section)] = std::move(dict);
            }
        }

        section_type m_sections;
    };  

    read_only_configuration parse_configuration(const char* filename)
    {
        std::fstream ifs(filename, std::ios_base::in | std::ios_base::binary);
        
        auto context = std::make_unique<std::string>(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

        auto sections = *context
            | std::views::split(detail::nl)
            | std::views::transform(detail::get_section_or_entry_context)
            | std::views::filter(std::ranges::size)
            | std::views::chunk_by(detail::chunk_section_and_entries)  
            | std::views::transform(detail::to_section);

        std::unordered_map<std::string_view, std::unordered_map<std::string_view, std::string_view>> result;
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

