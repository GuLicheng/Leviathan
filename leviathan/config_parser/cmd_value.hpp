#pragma once

#include <algorithm>
#include <iostream>
#include <format>
#include <memory>
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
        only_one,      
    };

    namespace scanner
    {
        inline constexpr auto only_one = []<typename I, typename S>(I first, S last) static {
            std::cout << std::format("Key = {}, value = {}\n", *first, *++first);
            return std::ranges::next(first, 2, last);
        };
    }

    struct cmd_error : std::runtime_error
    { using std::runtime_error::runtime_error; }; 

    class option
    {
        // friend class option_builder;
        friend class options;

        class builder
        {
            std::unique_ptr<option> m_op;

            builder() = default;

            template <typename... SNames>
            option_builder& name(SNames... sname) &&
            {
                (m_op->m_name_or_flags.emplace_back(sname.get()), ...);
                return *this;
            }

            option_builder& help(std::format_args info) &&
            {
                m_op->m_help = info.get();
                return *this;
            }

            option_builder& type(option_type tp) &&
            {
                m_op->m_tp = tp;
                return *this;
            }

            option build() &&
            {
                return std::move(*m_op);
            }
        };

        builder make() 
        {
            return {};
        }

private:

        std::vector<std::string_view> m_name_or_flags;
        std::string_view m_help;
        option_type m_tp;

        constexpr bool is_name_or_flag(std::string_view name) const
        {
            return std::ranges::contains(m_name_or_flags, name);
        } 

        template <typename I, typename S>
        constexpr I parse(I first, S last) const
        {
            if (first == last || !is_name_or_flag(*first))
            {
                return first;
            }

            using enum option_type;
            switch (m_tp)
            {
                case only_one: return scanner::only_one(first, last);
                default: throw cmd_error("Not implemented");
            }
        }
    };

    class parse_result
    {

    private:

        std::vector<std::string_view> m_names;
        std::vector<cmd_value>  m_values;
    };
    
} // namespace leviathan::config::cmd
