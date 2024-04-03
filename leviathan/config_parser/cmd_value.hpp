#pragma once

#include <utility>
#include <format>
#include <string_view>
#include <string>
#include <cstdint>
#include <vector>

namespace leviathan::config::cmd
{
    class cmd_value
    {
    public:

        bool check_is_empty() const
        { return arg_count() == 0; }

        size_t arg_count() const
        { return m_last - m_first; }

        // ("-n", "CSharpApplication", "OtherParameters")
        std::string to_string() const
        {
            std::string s;
            const char* delimiter = '(';

            for (auto p = m_first; p != m_last; ++p)
            {
                std::format_to(std::back_inserter(s), "{}{}", std::exchange(delimiter, ", "), *p);
            }

            s += ')';
            return s;
        }
    
    private:
        // For follow instructions:
        // dotnet new console -n CSharpApplication
        // We store two pointers, the first point to 'CSharpApplication' and 
        // the second point to the next position of 'CSharpApplication'
        const char** m_first;
        const char** m_last;

        std::string_view m_name; // store longname or shortname
    };

    enum struct option_type 
    {
        none,
        at_least_one,   
        just_one,      

        optional,                
        required,  // split required and non-required option

        required_none,
        required_at_least_one,
        required_just_one, 
    };

    struct cmd_error : std::runtime_error
    { using std::runtime_error::runtime_error; }; 

    class option
    {
        friend class option_builder;
        friend class options;

        std::string_view m_shortname;
        std::string_view m_longname;
        std::string_view m_help;
        option_type m_tp;

        constexpr bool is_name(std::string_view name) const
        {
            return name == m_longname || name == m_shortname;
        } 

        template <typename I, typename S>
        constexpr I parse(I first, S last) const
        {
            if (first == last)
            {
                return first;
            }

            std::string_view name = *first;

            if (name == m_longname || name == m_shortname)
            {
                using enum option_type;
                switch (m_tp)
                {
                    case just_one: return first + 1;
                    default: throw cmd_error("Not implemented");
                }
            }

            return first;
        }
    };

    class parse_result
    {

    private:

        std::vector<std::string_view> m_names;
        std::vector<cmd_value>  m_values;
    };
    
} // namespace leviathan::config::cmd
