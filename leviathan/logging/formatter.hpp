#pragma once

#include "common.hpp"

#include <format>
#include <iterator>
#include <string>
#include <array>

namespace cpp::logging
{
    struct logging_formatter_error : public std::logic_error 
    {
        using std::logic_error::logic_error;
    };

    class pattern_formatter : public basic_formatter
    {
        static constexpr std::string_view default_pattern = "%(time) : %(level) - %(message)";

        static constexpr std::array<std::string_view, 2> replace_table[] = {
            { "time", "0" },
            { "level", "1" },
            { "message", "2" },
            { "line", "3" },
            { "file", "4" },
        };

    public:

        std::string do_format(const record& r) override 
        { 
            auto message = std::vformat(m_pattern, 
                unmove_make_format_args(r.m_tp, level_to_string(r.m_level), r.m_message, r.m_sl.line(), r.m_sl.file_name()));

            if (m_newline)
            {
                message += '\n';
            }
            return message; 
        }

        std::string_view get_name() const override
        { return m_name; }

        explicit pattern_formatter(std::string name = "pattern_formatter", 
            std::string_view pattern = default_pattern, bool newline = true)
            : m_name(name), m_newline(newline)
        {
            replace_pattern(pattern);
        }

    private:

        struct parser
        {
            const char* m_curr;
            const char* m_sent;
            std::string m_result;

            std::string parse()
            {
                m_result.clear();
                for (;m_curr != m_sent; ++m_curr)
                {
                    const char ch = *m_curr;
                    switch (ch)
                    {
                        case '\\': parse_escape(); break;
                        case '%': parse_context(); break;
                        default: m_result += ch;
                    }
                }
                return m_result;
            }

            void parse_context()
            {
                ++m_curr;
                if (*m_curr != '(')
                    throw logging_formatter_error("Expected ( after %");
                ++m_curr;
                m_result += '{';
                parse_keyword();
            }

            void parse_keyword()
            {
                for (auto [keyword, idx] : replace_table)
                {
                    if (parse_keyword(keyword))
                    {
                        m_result += idx;
                        m_result += '}';
                        return;
                    }
                }
                throw logging_formatter_error("Expected one of keyword in (message|time|level|line|file)");
            }

            bool parse_keyword(std::string_view keyword)
            {
                // (message) => message)
                if (size_t(m_sent - m_curr) < keyword.size() + 1)
                    return false;

                std::string_view context(m_curr, keyword.size());

                if (context != keyword || *(m_curr + keyword.size()) != ')')
                    return false;

                m_curr += keyword.size();

                return true;
            }

            void parse_escape()
            {
                ++m_curr;
                if (m_curr == m_sent) 
                {
                    throw logging_formatter_error("Escape character cannot at the end");
                } 
                m_result += *m_curr;
            }

        };

        void replace_pattern(std::string_view input)
        {
            parser p(input.data(), input.data() + input.size());
            m_pattern = p.parse();
        }

        std::string m_name;
        std::string m_pattern;
        bool m_newline;
    };

    class default_formatter : public pattern_formatter
    {
    public:

        default_formatter() : pattern_formatter("default") { }

    };

    // For debugging
    class null_formatter : public basic_formatter
    {
    public:

        null_formatter() = default;

        std::string do_format(const record&) override 
        { return "null\n"; }

    };

} // formatters
