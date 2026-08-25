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

template <typename ErrorCode, typename CharT>
struct std::formatter<nom::context<ErrorCode>, CharT> : public std::formatter<std::string_view, CharT>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& pc)
    {
        return std::formatter<std::string_view, CharT>::parse(pc);
    }

    template <typename FormatContext>
    auto format(const nom::context<ErrorCode>& ctx, FormatContext& fc) const
    {
        return std::formatter<std::string_view, CharT>::format(static_cast<std::string_view>(ctx), fc);
    }
};

