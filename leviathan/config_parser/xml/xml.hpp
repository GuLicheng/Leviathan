// https://learn.microsoft.com/zh-cn/previous-versions/dotnet/netframework-4.0/ms256177(v=vs.100)

/*
    Elements: 
        - <elementName att1Name="att1Value" att2Name="att2Value"...> </elementName>
        - <elementName att1Name="att1Value" att2Name="att2Value".../>
    Prolog:
        - <?xml version="1.0" encoding="UTF-8"?>
    Comment:
        - <!--- <test pattern="SECAM" /><test pattern="NTSC" /> -->
    Character Reference:
        - &lt '<'
        - &gt '>'
        - &amp '&'
        - &apos '''
        - &quot '"'
    CDATA:
        - <![CDATA[An in-depth look at creating applications with XML, using <, >,]]>
    Attribute:
        - <myElement contraction="isn't" />
        - <myElement question='They asked "Why?"' />
*/  

#pragma once

#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/common.hpp>
#include <leviathan/config_parser/parse_context.hpp>

#include <expected>

namespace leviathan::config::xml
{
    
using attribute = std::pair<std::string, std::string>;

class element
{
    std::vector<attribute> m_attributes;
    
    union
    {
        std::string m_text;
        std::vector<element> m_children;
    };

    bool m_is_text;

    std::unique_ptr<element> m_sibling;
};

enum class error_code
{
    succeed,
    no_declaration,
    unknown_declaration,
    no_element,
    eof,
};

class document
{
    struct parser
    {
        parser(std::string_view context, document& doc) 
            : m_ctx(context), m_doc(doc) { }

        document operator()()
        {
            auto declaration = parse_declaration();
            auto root = parse_element();
        }     

        std::expected<element, error_code> parse_declaration()
        {
            error_code ec = error_code::succeed;
            auto idx = m_ctx.search("<?xml");

            if (idx == m_ctx.npos)
            {
                ec = error_code::no_declaration;
            }
            else
            {
                if (!m_ctx.consume(' '))
                {
                    ec = error_code::unknown_declaration;
                }
                else
                {
                    parse_attribute();
                }
            }
        }

        std::expected<element, error_code> parse_element()
        {

        }

        parse_context m_ctx;
        document& m_doc;
    };

public:

    document() = default;

    void load(const char* filename)
    {
        auto context = read_file_context(filename);
        parser(context, *this)();
    }

private:

    element m_declaration;
    element m_root;
};

} // namespace leviathan::config::xml

