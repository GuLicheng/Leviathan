#pragma once

#include <type_traits>
#include <string_view>
#include <chrono>
#include <string>
#include <source_location>
#include <memory>
#include <utility>

namespace cpp::logging
{
    template <typename... Args>
    auto unmove_make_format_args(Args&&... args)
    {
        return std::make_format_args(args...);
    }

    class logger;

    class basic_formatter;

    class basic_filter;

    class basic_handler;

    enum class level : int
    {
        debug    = 0,
        info     = 1,
        warning  = 2,
        error    = 3,
        critical = 4,
    };

    using level_underlying_type = std::underlying_type_t<level>;

    std::string_view level_to_string(level l)
    {
        static std::string_view names[] = 
        {
            "DEBUG",
            "INFO",
            "WARNING",
            "ERROR",
            "CRITICAL",
        };
        return names[static_cast<int>(l)];
    }

    using clock_type = std::chrono::system_clock;

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

        virtual std::string_view get_name() const = 0;

        virtual ~basic_formatter() = default;

    };

    class basic_filter
    {
    public:

        virtual bool do_filter(const record&) = 0;

        virtual std::string_view get_name() const = 0;

        virtual ~basic_filter() = default;

    };

    class basic_handler
    {
    public:
        
        virtual void do_handle(const record&) = 0;

        virtual std::string_view get_name() const = 0;

        virtual ~basic_handler() = default;

    };

    /**
     * @brief Component factory.
     * 
     * Return std::unique_ptr<Target> point to Source. 
     * 
     * @param Target target type.
     * @param Source origin type.
    */
    template <typename Target, typename Source = Target>
    struct factory
    {
        static_assert(std::is_base_of_v<Target, Source>);

        template <typename... Args>
        static std::unique_ptr<Target> make(Args&&... args)
        { return std::make_unique<Source>((Args&&) args...); }
    };

    template <typename Handler, typename... Args>
    auto make_handler(Args&&... args)
    { return factory<basic_handler, Handler>::make((Args&&) args...); }

    template <typename Formatter, typename... Args>
    auto make_formatter(Args&&... args)   
    { return factory<basic_formatter, Formatter>::make((Args&&) args...); }

    template <typename Filter, typename... Args>
    auto make_filter(Args&&... args)   
    { return factory<basic_filter, Filter>::make((Args&&) args...); }
    
    template <typename T, typename Deleter>
    std::shared_ptr<T> unique_to_shared(std::unique_ptr<T, Deleter> unique)
    { return std::move(unique); }

} 

