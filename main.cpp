#include <experimental/nom/nom.hpp>
#include <experimental/nom/parser.hpp>
#include <leviathan/type_caster.hpp>


// [section]

auto section_parser = nom::sequence::delimited(
    nom::sequence::preceded(
        nom::character::space0,
        nom::character::char_('[')
    ),
    nom::sequence::delimited(
        nom::character::space0,
        nom::branch::alt(
            nom::character::alphanumeric1,
            nom::character::one_of(".")
        ),
        nom::character::space0
    ),
    nom::sequence::terminated(
        nom::character::char_(']'),
        nom::character::space0
    )
);

int main(int argc, char const *argv[])
{

    std::string_view context = " [ Boris.Weapon.Anno ] ";

    auto result = section_parser(context);

    // if (result)
    // {
    //     std::print("Parsed section: '{}'\n", *result);
    //     std::print("Remaining context: '{}'\n", context);
    // }
    // else
    // {
    //     std::print("Failed to parse section: {}\n", result.error().info);
    // }

    return 0;
}
