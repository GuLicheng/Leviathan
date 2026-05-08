#include <leviathan/config_parser/json/json.hpp>
#include <iostream>
#include <meta>
#include <print>
#include "annotations.hpp"
#include <string>
#include <algorithm>
#include <leviathan/extc++/format.hpp>
#include <ranges>
#include <array>

enum class Gender
{
    Male [[=cpp::refl::uppercase]], 
    
    Female
};

template <> struct std::formatter<Gender> : cpp::universal_enum_formatter<Gender> { };

struct Coordinate
{
    int X;
    int Y;
};

template <>
inline constexpr bool cpp::use_default_caster<Coordinate> = true;

template <> struct std::formatter<Coordinate> : cpp::universal_formatter { };

struct Student
{
    [[=cpp::refl::uppercase, =cpp::refl::lowercase]]
    int Age;

    [[=cpp::refl::lowercase, =cpp::refl::longname]]
    std::string Name;

    // [[=cpp::refl::ignore]]
    Gender Gender;
    
    std::vector<int> Scores;

    Coordinate coord;
};

template <>
inline constexpr bool cpp::use_default_caster<Student> = true;

template <> struct std::formatter<Student> : cpp::universal_formatter { };



void Test()
{
    cpp::json::value root = {
        {"Age", 20},
        {"Name", "Alice"},
        {"Gender", "Male"},
        {"Scores", {90, 85, 78}},
        {"coord", {{"X", 10}, {"Y", 20}}}
    };

    // auto student = parse<Student>(root);
    auto student = cpp::cast<Student>(root);

    std::print("Student: {}\n", student);
}

int main() 
{
    Test();
    std::println("{}", has_identifier(std::meta::reflect_constant(5)));
}


