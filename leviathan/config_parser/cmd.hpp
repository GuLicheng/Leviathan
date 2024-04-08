// CommandLine

#pragma once

#include "cmd_value.hpp"

namespace leviathan::config::cmd
{
    class options
    {
    public:

        options() = default;

        void add_option(option op) 
        { m_options.emplace_back(op); }

        void add_suboption(options subop)
        { m_suboptions.emplace_back(std::move(subop)); }

        parse_result parse(int argc, const char* argv[]) const
        {

        }

    private:

        const char** parse_value(const char** first, const char** last)
        {
            parse_result result;

            if (first == last)
            {
                return first;
            }

            std::string_view name = *first;
            
            if (name.starts_with('-'))
            {
                auto parse_option_only = [this, &](const char* first, const char* last) {
                    if (first == last)
                    {
                        return first;
                    }

                    for (auto cur = first; cur != last; ++cur)
                    {
                        for (const auto& op : m_options)
                        {
                            
                        }
                    }

                };

                return parse_option_only(first, last);
            }
            else
            {
                auto it = std::ranges::find_if(m_suboptions, [=](const auto& op) {
                    return op.is_name(name);
                });

                if (it == m_suboptions.end())
                {
                    throw cmd_error(std::format("Unknown suboptions {}.", name));
                }

                return it->parse_value(first, last);
            }
        }

        parse_result m_result;
        std::vector<option> m_options;
        std::vector<options> m_suboptions;
    };

    class option_builder
    {
    public:

        static option_builder make() 
        { return option_builder(); }

        option build() &&
        { return std::move(m_op); }


    private:

        option m_op;
    };

} // namespace leviathan::parser






