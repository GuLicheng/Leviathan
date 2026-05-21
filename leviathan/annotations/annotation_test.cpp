#include <catch2/catch_all.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/annotations/all.hpp>

enum class 
[[=cpp::derive::debug]]
Color
{
    Red,
    Green [[=cpp::refl::rename("green")]],
    Blue
};

bool CheckEnumToString()
{
    return std::format("{}", Color::Red) == "Red" &&
           std::format("{}", Color::Green) == "green" &&
           std::format("{}", Color::Blue) == "Blue";
}

TEST_CASE("Enum to string conversion with debug annotation", "[annotations]")
{
    REQUIRE(CheckEnumToString());
}