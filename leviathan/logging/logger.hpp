#pragma once

#include "common.hpp"
#include "filter.hpp"
#include "formatter.hpp"
#include "handler.hpp"

#include <vector>
#include <optional>
#include <ranges>
#include <mutex>
#include <map>
#include <assert.h>

namespace leviathan::logging
{
    class logger : public std::enable_shared_from_this<logger>
    {
        struct format_string_with_source_location
        {
            std::string_view m_value;
            std::source_location m_sl;

            template <typename T>
                requires std::convertible_to<const T&, std::string_view>
            consteval format_string_with_source_location(const T& str, const std::source_location sl = std::source_location::current())
                : m_value(str), m_sl(sl) { }
        };

    public:

        logger(std::string name, level lv = level::Info) 
            : m_name(std::move(name)), m_level(lv) { }

        template <typename... Args>
        void info(format_string_with_source_location fmt, Args&&... args) 
        { log(level::Info, fmt.m_value, fmt.m_sl, (Args&&) args...); }

        template <typename... Args>
        void debug(format_string_with_source_location fmt, Args&&... args) 
        { log(level::Debug, fmt.m_value, fmt.m_sl, (Args&&) args...); }

        template <typename... Args>
        void warn(format_string_with_source_location fmt, Args&&... args) 
        { log(level::Warning, fmt.m_value, fmt.m_sl, (Args&&) args...); }

        template <typename... Args>
        void error(format_string_with_source_location fmt, Args&&... args) 
        { log(level::Error, fmt.m_value, fmt.m_sl, (Args&&) args...); }

        template <typename... Args>
        void critical(format_string_with_source_location fmt, Args&&... args) 
        { log(level::Critical, fmt.m_value, fmt.m_sl, (Args&&) args...); }

        void add_handler(std::unique_ptr<basic_handler> h)
        { m_handlers.emplace_back(std::move(h)); }

        void remove_handler(std::string_view name)
        {
            // What if there are more one handler match the result?
           auto it = std::ranges::find_if(m_handlers, [=](const auto& f) {
                return f->get_name() == name;
            });
            if (it != m_handlers.end())
                m_handlers.erase(it);
        }

        void set_level(level lv) { m_level = lv; }

        void set_filter(std::unique_ptr<basic_filter> f)
        { m_filter = std::move(f); }

    private:

        void write()
        {
            auto invalid_records = m_records | std::views::filter([this](const auto& r) {
                bool level_ok = r.m_level >= m_level;
                if (m_filter)
                    level_ok = level_ok && m_filter->do_filter(r);
                return level_ok;
            });

            // cartesian_product is much slower than for-loop
            std::ranges::for_each(
                std::views::cartesian_product(m_handlers, invalid_records),
                [](auto&& hr) { hr.first->do_handle(hr.second); }
            );
        }

        template <typename... Args>
        void log(level lv, std::string_view fmt, std::source_location sl, Args&&... args) 
        {
            /**
             * Can we just save args...? Some reference may be dangling.
             * E.g.
             *  logger = make_logger();
             *  {
             *      vector<T> v;
             *      ... 
             *      logger.info("", v[0]);
             *  }
             * 
             * Can we remove the low level message directly?(1 or 2)
             *  1. Only record the high level message. 
             *  2. Only report the high level message.
            */
            auto message = std::vformat(fmt, std::make_format_args(args...));
            m_records.emplace_back(std::move(message), lv, clock_type::now(), sl);
            // Write immediately or latter? If not immediately, some error happened
            // and process terminated, how can we write the rest message? RAII?
            write(); 
        }

        std::string m_name;
        level m_level;
        std::vector<std::unique_ptr<basic_handler>> m_handlers;
        std::vector<record> m_records;
        std::unique_ptr<basic_filter> m_filter;
    };
    
    class logger_manager 
    {
    public:

        inline static logger_manager* instance = nullptr;

        static std::shared_ptr<logger> get_root() 
        {
            static logger root("root");
            return root.shared_from_this();
        }

    private:

        std::map<std::string_view, std::shared_ptr<logger>> m_loggers;

    };

} // logger 




