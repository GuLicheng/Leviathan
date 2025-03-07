#pragma once

#include "../parse_context.hpp"
#include "document.hpp"

namespace leviathan::config::xml
{

class xml_parse_error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class decoder
{
public:

    decoder(std::string_view context) : m_ctx(context) { }

    element* parse();

    declaration parse_declaration()
    {
        // Try parse the declaration
        skip_whitespace(false);
        consume("<?xml");
        skip_whitespace(true);
        consume("version");
        skip_whitespace(false);
        consume("=");
        skip_whitespace(false);
        auto version = parse_string();

        // Try parse the encoding
        skip_whitespace(true);
        consume("encoding");
        skip_whitespace(false);
        consume("=");
        skip_whitespace(false);

        auto encoding = parse_string();
        skip_whitespace(false);
        consume("?>");

        // The version and encoding are short strings, so we can afford to copy them.
        return { .m_version = version, .m_encoding = encoding };
    }

    string parse_string()
    {
        // The string must started with ' or "
        char quote = m_ctx.peek();
        
        if (quote != '\'' && quote != '"')
        {
            throw xml_parse_error("Expected a quote character(' or \")");
        }
        
        m_ctx.advance(1);
    }

private:

    void skip_whitespace(bool require_blank)
    {
        if (require_blank && !::isblank(m_ctx.peek()))
        {
            throw xml_parse_error("Expected a blank character");
        }
        m_ctx.skip_whitespace();
    }   

    void consume(std::string_view str)
    {
        if (!m_ctx.consume(str))
        {
            throw xml_parse_error("Expected " + std::string(str));
        }
    }

    parse_context m_ctx;
};
    
} // namespace leviathan::config::xml

