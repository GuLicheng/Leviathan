#include <experimental/nom/nom.hpp>
#include <experimental/nom/parser.hpp>
#include <leviathan/type_caster.hpp>
#include <print>


// [section]

auto left_bracket = nom::sequence::preceded(
    nom::character::space0,
    nom::character::char_('[')
);

auto right_bracket = nom::sequence::terminated(
    nom::character::char_(']'),
    nom::character::space0
);

auto identifier = nom::sequence::delimited(
    nom::character::space0,
    nom::character::alphanumeric1,
    nom::character::space0
);

auto section_parser = nom::sequence::delimited(
    left_bracket,
    identifier,
    right_bracket
);

int main(int argc, char const *argv[])
{

    std::string_view context = " [ Boris0Weapon0Anno ] ";

    auto result = section_parser(context);

    if (result)
    {
        std::print("Parsed section: '{}'\n", *result);
        std::print("Remaining context: '{}'\n", context);
    }
    else
    {
        std::print("Failed to parse section: {}\n", result.error().info);
    }

    return 0;
}
