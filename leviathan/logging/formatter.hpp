#pragma once

#include "common.hpp"

#include <format>
#include <iterator>
#include <array>

namespace leviathan::logging
{
    class default_formatter : public basic_formatter
    {
    public:

        default_formatter(bool newline = true) : m_newline(newline) { }

        std::string do_format(const record& r) override 
        { 
            auto message = std::format(
                "{0:%F}-{0:%R} LEVEL-{1}: {2} with [file = {3}, line = {4}", 
                r.m_tp, level_to_string(r.m_level), r.m_message, r.m_sl.file_name(), r.m_sl.line());

            if (m_newline) message += '\n';
            return message; 
        }

    private:

        bool m_newline;

    };

    class pattern_formatter : public basic_formatter
    {

        constexpr static std::string_view default_pattern = "{time:%F} : %{level} - {message}";

        constexpr static std::array<std::string_view, 2> replace_table[] = {
            { "time", "1" },
            { "level", "2" },
            { "message", "3" },
        };

    public:

        explicit pattern_formatter(std::string_view pattern = default_pattern)
        {
            replace_pattern(pattern);
        }

    private:

        void replace_pattern(std::string_view input)
        {
            m_pattern = input;
            for (auto [k, v] : replace_table)
            {
            }
        }

        int index(std::string_view context)
        {
            auto iter = std::ranges::find(replace_table, context);
            return iter == std::ranges::end(replace_table) 
                ? -1 
                : std::ranges::distance(std::ranges::begin(replace_table), iter);
        }

        std::string m_pattern;

    };

    // For debugging
    class null_formatter : public basic_formatter
    {
    public:

        null_formatter() = default;

        std::string do_format(const record& r) override 
        { return "null\n"; }

    };

} // formatters
