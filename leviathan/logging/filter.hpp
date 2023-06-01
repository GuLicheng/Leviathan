#pragma once

#include "common.hpp"

namespace leviathan::logging
{
    class default_filter : public basic_filter
    {
    public:
        
        bool do_filter(const record& r) override { return true; }

    };

} // namespace leviathan::logging
