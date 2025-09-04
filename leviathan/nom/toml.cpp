#include <leviathan/nom/nom.hpp>
#include <leviathan/config_parser/context.hpp>
#include <leviathan/config_parser/toml/value.hpp>

namespace toml = cpp::config::toml;

using Context = cpp::config::context;

struct TomlValueParser
{
    toml::value parse(Context context)
    {
        auto parser = nom::branch::alt(
            nom::combinator::map(
                nom::bytes::tag("true"),
                []() static { return toml::make<toml::boolean>(true); }
            ),
            nom::combinator::map(
                nom::bytes::tag("false"),
                []() static { return toml::make<toml::boolean>(false); }
            )
        );
    }
};

struct TomlParser
{
    // [[array]]
    // [table]
    enum class State { Table, Array };
    
    void parse(Context context)
    {
        auto comment_consumer = nom::sequence::delimited(
            nom::sequence::preceded(
                nom::character::multispace0,
                nom::character::char_('#')
            ),
            nom::character::not_line_ending,
            nom::character::line_ending
        );

        auto section_name_parser = nom::sequence::delimited(
            nom::character::multispace0,
            nom::bytes::take_while1([](char c) { 
                return std::isalnum(c) || c == '.' || c == '_'; 
            }),
            nom::character::multispace0
        );

        auto table_section_parser = nom::sequence::delimited(
            nom::sequence::preceded(
                nom::character::char_('['),
                nom::character::multispace0
            ),

        )
    }
};

int main(int argc, char const *argv[])
{
    
    return 0;
}




