#pragma once

#include <leviathan/config_parser/json/value.hpp>
#include <leviathan/config_parser/core/error.hpp>
#include <leviathan/config_parser/core/parsers.hpp>
#include <leviathan/config_parser/core/context.hpp>

namespace cpp::config::json
{

namespace detail
{

struct value_parser
{
    using json_result = parse_result<value>;
    using stream = config::parse_context<typename json_result::error_type>;

    


};


}  // namespace detail




}  // namespace cpp::config::json




