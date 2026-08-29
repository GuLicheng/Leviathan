#pragma once

#include <leviathan/config_parser/context.hpp>

namespace winnow
{

template <typename ErrorCode>
class stream : public cpp::config::context
{

public:

    using error_code = ErrorCode;

    using cpp::config::context::context;

};

} // namespace winnow

template <typename ErrorCode, typename CharT>
struct std::formatter<winnow::stream<ErrorCode>, CharT> : public std::formatter<std::string_view, CharT>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& pc)
    {
        return std::formatter<std::string_view, CharT>::parse(pc);
    }

    template <typename FormatContext>
    auto format(const winnow::stream<ErrorCode>& ctx, FormatContext& fc) const
    {
        return std::formatter<std::string_view, CharT>::format(static_cast<std::string_view>(ctx), fc);
    }
};

