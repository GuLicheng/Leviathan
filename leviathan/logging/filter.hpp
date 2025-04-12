#pragma once

#include "common.hpp"

namespace cpp::logging
{
    class default_filter : public basic_filter
    {
    public:
        
        default_filter(std::string_view name = "default_filter")
            : m_name(name) { }

        bool do_filter(const record&) override { return true; }

    private:

        std::string m_name;

    };

} // namespace cpp::logging
