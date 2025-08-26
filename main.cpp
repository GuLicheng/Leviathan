#include <experimental/nom/nom.hpp>
#include <experimental/nom/parser.hpp>
#include <leviathan/type_caster.hpp>
#include <print>
#include <leviathan/extc++/file.hpp>
#include <leviathan/extc++/ranges.hpp>

class InIParser
{
public:

    InIParser() = default;

    auto parse(std::string_view context)
    {
        // ';' + Anycontext but linefeed

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
                nom::combinator::opt(
                    nom::sequence::delimited(
                        nom::character::char_(';'),
                        nom::bytes::take_till([](char c) { return c == '\n'; }),
                        nom::character::newline
                    )
                )
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

        auto value_parser = nom::sequence::delimited(
            nom::character::multispace0,
            identifier,
            nom::sequence::terminated(
                nom::character::multispace0,
                nom::combinator::opt(
                    nom::sequence::delimited(
                        nom::character::char_(';'),
                        nom::bytes::take_till([](char c) { return c == '\n'; }),
                        nom::character::newline
                    )
                )
            )
        );

        auto entry_parser = nom::multi::many0( 
            nom::sequence::separated_pair(
                key_parser,
                nom::character::char_('='),
                value_parser
            )
        );

        auto parser = nom::multi::many0(
            nom::sequence::pair(
                section_parser,
                nom::combinator::opt(entry_parser)
            )
        );

        auto result = parser(context);

        using ResultDictionary = std::unordered_map<
            std::string, 
            std::unordered_map<std::string, std::string>    
        >;
    
        if (!result)
        {
            throw std::runtime_error(std::format("Parse error: {}", (int)result.error().code));
        }

        return *result | cpp::views::pair_transform(
            cpp::cast<std::string>,
            [](auto optEntries) 
            { 
                return *optEntries 
                     | cpp::views::pair_transform(cpp::cast<std::string>, cpp::cast<std::string>) 
                     | std::ranges::to<std::unordered_map<std::string, std::string>>();
            }
        ) | std::ranges::to<ResultDictionary>();
    }
};

int main(int argc, char const *argv[])
{
    InIParser parser;

    auto result = parser.parse(std::string_view(R"(
        [SectionOne] ; comment
        key1=value1 ; comment
        key2 = value2
                    ; comment
        [SectionTwo]
        keyA=valueA
        keyB = valueB

        [Boris.Weapons]
        Name=AK47
        Damage=30
        Range=300
        Ammo=1
    )"));

    std::print("{}\n", result);

    return 0;
}
