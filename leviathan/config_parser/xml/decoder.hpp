#pragma once

#include <leviathan/config_parser/core/error.hpp>
#include <leviathan/config_parser/core/parsers.hpp>
#include <leviathan/config_parser/xml/node.hpp>

#include <optional>

namespace cpp::config::xml
{

using node_result = parse_result<node>;

struct cdata_decoder
{
    template <typename Context> 
    static constexpr std::optional<string> operator()(Context& context)
    {
        auto result = config::parser::block_comment("<![CDATA[", "]]>")
                     .map(cpp::elements<1>)
                     .operator()(context);

        if (!result)
        {
            return std::nullopt;
        }

        return string(result.value());
    }
};

template <typename Context> class decoder;

} // namespace cpp::config::xml
