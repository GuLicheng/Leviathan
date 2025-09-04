#include <leviathan/nom/nom.hpp>
#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/string.hpp>
#include <print>
#include <leviathan/extc++/file.hpp>
#include <leviathan/extc++/ranges.hpp>
#include <leviathan/config_parser/json/encoder.hpp>
#include <leviathan/config_parser/context.hpp>

using Context = cpp::config::context;

class InIParser
{
public:

    InIParser() = default;

    static std::string trim_str(Context sv)
    {
        return std::string(cpp::string::trim(sv.to_string_view()));
    }

    auto parse(Context context)
    {
        // ';' + Anycontext but linefeed
        auto comment_consumer = nom::sequence::delimited(
            nom::sequence::preceded(
                nom::character::multispace0,
                nom::character::char_(';')
            ),
            nom::character::not_line_ending,
            nom::character::line_ending
        );

        // [section] -> '[' + section_name + ']' ; comment
        auto left_bracket = nom::sequence::delimited(
            nom::character::multispace0,
            nom::character::char_('['),
            nom::character::multispace0
        );

        auto right_bracket = nom::sequence::delimited(
            nom::character::multispace0,
            nom::character::char_(']'),
            nom::sequence::terminated(
                nom::character::multispace0,
                nom::combinator::opt(comment_consumer)
            )
        );

        auto identifier = nom::bytes::take_while1([](char c) { 
            return std::isalnum(c) || c == '.'; 
        });

        auto section_parser = nom::sequence::delimited(
            left_bracket,
            identifier,
            right_bracket
        );

        // key = value
        auto key_parser = nom::sequence::delimited(
            nom::character::multispace0,
            identifier,
            nom::character::multispace0
        );

        static std::string_view valid_value_chars = 
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789"
            "`~!@#$%^&*()-_=+[{]}\\|:'\",<.>/? ";

        auto value_parser = nom::sequence::delimited(
            nom::character::multispace0,
            nom::bytes::take_while1([](char c) { return valid_value_chars.contains(c); }),
            nom::sequence::terminated(
                nom::character::multispace0,
                nom::combinator::opt(comment_consumer)
            )
        );

        auto entry_parser = nom::multi::many0( 
            nom::sequence::separated_pair(
                key_parser,
                nom::character::char_('='),
                value_parser
            )
        );

        // FIXME: comment at the begin of text.
        auto parser2 = nom::multi::many0(
            nom::sequence::pair(
                section_parser,
                nom::combinator::opt(entry_parser)
            )
        );

        auto parser = nom::sequence::preceded(
            nom::multi::many0(comment_consumer),
            parser2
        );

        auto result = parser(context);
    
        if (!result)
        {
            throw std::runtime_error(std::format("Parse error: {}", (int)result.error().code));
        }

        using ResultDictionary = std::unordered_map<
            std::string, 
            std::unordered_map<std::string, std::string>    
        >;

        auto temp = result->second | cpp::views::pair_transform(
            [] (Context ctx) static { return std::string(ctx.to_string_view()); },
            [](auto optEntries) static { 
                return *optEntries | 
                // return optEntries  // Ok if std::optional given range support. P3168R2 what about expected?
                       cpp::views::pair_transform(&InIParser::trim_str, &InIParser::trim_str) |
                       std::ranges::to<std::unordered_map<std::string, std::string>>();
            }
        ) | std::ranges::to<ResultDictionary>();

        return cpp::cast<cpp::json::value>(temp);
    }
};

int main(int argc, char const *argv[])
{
    InIParser parser;
    auto context = cpp::read_file_context(R"(D:\Library\Leviathan\test.ini)");
    auto result = parser.parse(Context(context.c_str()));

    std::print("{:4}\n", result);

    return 0;
}