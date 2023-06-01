#pragma once

#include <type_traits>
#include <string_view>
#include <chrono>
#include <string>
#include <source_location>
#include <memory>
#include <utility>

namespace leviathan::logging
{
    class logger;

    class basic_formatter;

    class basic_filter;

    class basic_handler;

    enum class level : int
    {
        Debug    = 0,
        Info     = 1,
        Warning  = 2,
        Error    = 3,
        Critical = 4,
    };

    using level_underlying_type = std::underlying_type_t<level>;

    std::string_view level_to_string(level l)
    {
        static std::string_view names[] = {
            "DEBUG",
            "INFO",
            "WARNING",
            "ERROR",
            "CRITICAL",
        };
        return names[static_cast<int>(l)];
    }

    using clock_type = std::chrono::gps_clock;

    struct record
    {
        std::string m_message;
        level m_level;
        typename clock_type::time_point m_tp;
        std::source_location m_sl;
    };

    class basic_formatter 
    {
    public:

        virtual std::string do_format(const record&) = 0;

        virtual ~basic_formatter() = default;

    };

    class basic_filter
    {
    public:

        virtual bool do_filter(const record&) = 0;

        virtual ~basic_filter() = default;

    };

    class basic_handler
    {
    public:
        
        virtual void do_handle(const record&) = 0;

    };

    template <typename Target, typename Source, typename... Args>
    std::unique_ptr<Target> make(Args&&... args)
    {
        static_assert(std::is_base_of_v<Target, Source>);
        return std::make_unique<Source>((Args&&) args...);
    }

    template <typename Handler, typename... Args>
    auto make_handler(Args&&... args)
    {
        return make<basic_handler, Handler>((Args&&) args...);
    }

    template <typename Formatter, typename... Args>
    auto make_formatter(Args&&... args)   
    {
        return make<basic_formatter, Formatter>((Args&&) args...);
    }

    template <typename Filter, typename... Args>
    auto make_filter(Args&&... args)   
    {
        return make<basic_filter, Filter>((Args&&) args...);
    }
    
    template <typename T, typename... Args>
    auto make_component(Args&&... args)
    {
        constexpr auto is_handler = std::is_base_of_v<basic_handler, T>;
        constexpr auto is_filter = std::is_base_of_v<basic_filter, T>;
        constexpr auto is_formatter = std::is_base_of_v<basic_formatter, T>;

        static_assert(is_handler + is_filter + is_formatter == 1);

        if constexpr (is_handler)
        {
            return make_handler((Args&&) args...);
        }
        else if constexpr (is_filter)
        {
            return make_filter((Args&&) args...);
        }
        else if constexpr (is_formatter)
        {
            return make_formatter((Args&&) args...);
        }
        else
        {
            std::unreachable();
        }
    }

} 

