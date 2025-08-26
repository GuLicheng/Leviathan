#include <experimental/nom/nom.hpp>
#include <experimental/nom/parser.hpp>
#include <leviathan/type_caster.hpp>
#include <print>


// [section] -> '[' + section_name + ']'

auto left_bracket = nom::sequence::delimited(
    nom::character::multispace0,
    nom::character::char_('['),
    nom::character::multispace0
);

auto right_bracket = nom::sequence::delimited(
    nom::character::multispace0,
    nom::character::char_(']'),
    nom::character::multispace0
);

auto identifier = nom::character::alphanumeric1;

auto section_parser = nom::sequence::delimited(
    left_bracket,
    identifier,
    right_bracket
);

// key = value
auto key_parser = nom::sequence::delimited(
    nom::character::multispace0,
    nom::character::alphanumeric1,
    nom::character::multispace0
);

auto value_parser = nom::sequence::delimited(
    nom::character::multispace0,
    nom::character::alphanumeric1,
    nom::character::multispace0
);

auto entry_parser = nom::sequence::separated_pair(
    key_parser,
    nom::character::char_('='),
    value_parser
);


int main(int argc, char const *argv[])
{

    std::string_view context1 = " [ Boris0Weapon0Anno ] ";

    auto result = section_parser(context1);

    if (result)
    {
        std::print("Parsed section: '{}'\n", *result);
        std::print("Remaining context1: '{}'\n", context1);
    }
    else
    {
        std::print("Failed to parse section: {}\n", result.error().info);
    }

    auto context2 = std::string_view(" key1 = value1 \n  ");
    auto result2 = entry_parser(context2);

    if (result2)
    {
        auto [key, value] = *result2;
        std::print("Parsed entry: key='{}', value='{}'\n", key, value);
        std::print("Remaining context2: '{}'\n", context2);
    }
    else
    {
        std::print("Failed to parse entry: {}\n", result2.error().info);
    }

    return 0;
}
