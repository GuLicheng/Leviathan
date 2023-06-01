#pragma once

#include "common.hpp"

#include <format>

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
} // formatters
