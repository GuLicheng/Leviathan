#pragma once

#include <leviathan/config_parser/context.hpp>

namespace nom
{

template <typename ErrorCode>
class context : public cpp::config::context
{

public:

    using error_code = ErrorCode;

    using cpp::config::context::context;

};

} // namespace nom
