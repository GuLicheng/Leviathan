#pragma once

#include <leviathan/config_parser/context.hpp>

namespace nom
{

template <typename ErrorCode>
class context : public cpp::config::cursor_context
{

public:

    using error_code = ErrorCode;

    using cpp::config::cursor_context::cursor_context;

};

} // namespace nom
