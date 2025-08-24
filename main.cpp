#include <experimental/nom/combinator.hpp>
#include <experimental/nom/parser.hpp>
#include <leviathan/type_caster.hpp>
#include <print>

struct Color
{
    int r;
    int g;
    int b;

    static Color from_string(std::string_view& ctx)
    {
        auto r = cpp::cast<int>(ctx.substr(0, 2), 16);
        auto g = cpp::cast<int>(ctx.substr(2, 2), 16);
        auto b = cpp::cast<int>(ctx.substr(4, 2), 16);
        ctx.remove_prefix(6);
        return Color{ r, g, b };
    }
};

auto parse_literal(std::string_view& ctx)
{
    return nom::pair( 
        nom::value(true, nom::tag("True")),
        nom::value(false, nom::tag("False"))
    )(ctx);
}

int main(int argc, char const *argv[])
{
    std::string_view input = "TrueFalse|||";

    auto result = parse_literal(input);

    if (result)
    {
        auto [v1, v2] = *result;
        std::print("Parsed values: {}, {}\n", v1, v2);
    }
    else
    {
        std::print("Error: {}\n", result.error().info);
    }

    std::print("Remaining input: '{}'\n", input);

    return 0;
}
