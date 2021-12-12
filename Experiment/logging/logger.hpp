#pragma once

#include "log_record.hpp"
#include <string_view>
#include <tuple>

namespace leviathan::logging
{

    template <typename... Handlers>
    class logger
    {
        std::tuple<Handlers...> m_handlers;

        

    };


}