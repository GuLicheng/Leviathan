#pragma once

#include "common.hpp"
#include "formatter.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <leviathan/meta/template_info.hpp>

namespace leviathan::logging
{
    class default_handler : public basic_handler
    {
    public:

        default_handler(std::string_view name, std::unique_ptr<basic_formatter> fmt)
            : m_name(name), m_formatter(std::move(fmt)) 
        { }

        void set_formatter(std::unique_ptr<basic_formatter> fmt)
        { m_formatter = std::move(fmt); }

        std::string_view get_name() const override
        { return m_name; }

    protected:

        bool is_valid_record(const record& rd) 
        {
            return m_filters.empty() 
                    ? true 
                    : std::ranges::all_of(m_filters, [&](auto&& f) { return f->do_filter(rd); });
        }

        std::string m_name;
        std::vector<std::unique_ptr<basic_filter>> m_filters;
        std::unique_ptr<basic_formatter> m_formatter;
    };

    class console_handler : public default_handler
    {
    public:

        console_handler(std::string_view name, std::ostream& os) 
            : default_handler(name, make_formatter<default_formatter>()),m_os(os) 
        { }

        console_handler(std::string_view name, std::unique_ptr<basic_formatter> fmt, std::ostream& os) 
            : default_handler(name, std::move(fmt)), m_os(os)
        { }

        void do_handle(const record& rd) override
        {
            if (is_valid_record(rd))
            {
                auto message = m_formatter->do_format(rd);
                m_os << message;
            }
        }

    private:

        std::ostream& m_os;

    };

    class file_handler : public default_handler
    {
    public:
    
        file_handler(std::string_view name, std::string_view filename)
            : default_handler(name, make_formatter<default_formatter>()), m_filename(filename)
        { }

        void do_handle(const record& rd) override
        {
            std::ofstream ofs(m_filename, std::ios::out | std::ios::app);

            if (is_valid_record(rd))
            {
                auto message = m_formatter->do_format(rd);
                ofs << message;
            }
        }

    private:
        
        std::string m_filename;

    };
}
