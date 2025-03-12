#pragma once

#include "../parse_context.hpp"
#include "document.hpp"
#include <stack>

namespace leviathan::config::xml
{

class xml_parse_error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class parser
{
    inline static std::pair<std::string_view, char> entities[] = {
        { "lt", '<' },
        { "gt", '>' },
        { "amp", '&' },
        { "apos", '\'' },
        { "quot", '"' },
    };

public:

    parser(std::string_view context) : m_ctx(context) { }

    element* parse_element(element* parent)
    {
        skip_whitespace(false);
        match_and_consume('<');

        auto tag_name = parse_start_tag();
        auto ele = allocate_and_construct<element>(std::move(tag_name), parent);
        std::unique_ptr<element> guard(ele);
        auto attributes = parse_attributes();

        if (attributes != nullptr)
        {
            ele->m_attributes = attributes;
        }
        
        m_stack.emplace(ele);
        skip_whitespace(false);

        if (current() == '/')
        {
            // <name attr1="value1" attr2="value2"/>
            match_and_consume('/');
            match_and_consume('>');
            ele->m_children_or_text.emplace<string>("");
            // return ele;
            return guard.release();
        }
        else
        {
            // <name attr1="value1" attr2="value2">...</name>
            match_and_consume('>');

            auto body = parse_element_body();

            match_and_consume('<');
            match_and_consume('/');
            
            auto tag_name = parse_end_tag();

            if (tag_name != ele->m_tag)
            {
                throw xml_parse_error("Tag name mismatch");
            }
            
            match_and_consume('>');
            // return ele;
            return guard.release();
        }
    }

    body parse_element_body()
    {
        // Parse the element body
        if (current() == '<')
        {
            return parse_element();
        }
        else
        {
            return parse_text();
        }
    }

    attribute_list* parse_attributes()
    {
        // while ()
    }

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

    string parse_start_tag()
    {
        skip_whitespace(false);
        consume("<");
        auto tag = parse_string();
        consume(">");
        return tag;
    }

    string parse_context();

    string parse_string()
    {
        // The string must started with ' or "
        char quote = m_ctx.current();
        string retval = "";
        
        if (quote != '\'' && quote != '"')
        {
            throw xml_parse_error("Expected a quote character(' or \")");
        }
        
        m_ctx.advance_unchecked(1);
       
        while (m_ctx && m_ctx.current() != quote)
        {
            if (m_ctx.current() == '&')
            {
                m_ctx.advance_unchecked(1);

                // Try to parse the entity
                for (auto [entity, value] : entities)
                {
                    if (try_comsume(entity))
                    {
                        retval += value;
                        break;
                    }
                }

                throw xml_parse_error("Unknown entity");
            }
            else
            {
                // Append the character to the string
                retval += m_ctx.current();
            }
        }

        return retval;
    }

    document operator()()
    {
        auto declaration = parse_declaration();
        auto root = parse_element();
        return { declaration, root };
    }

private:

    char current() const
    {
        return m_ctx.current();
    }

    void match_and_consume(char c)
    {
        if (current() != c)
        {
            throw xml_parse_error(std::format("Expected '{}' but got {}", c, current()));
        }
    }

    bool try_comsume(std::string_view s)
    {
        return m_ctx.consume(s);
    }

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

    std::stack<std::string_view> m_stack;  // store the tag name
};
    
} // namespace leviathan::config::xml

