#include <print>
#include <iostream>
#include <leviathan/extc++/meta.hpp>
#include "leviathan/annotations.hpp"
#include <string>
#include <algorithm>
#include <leviathan/extc++/format.hpp>
#include <ranges>
#include <array>

enum class Gender 
{
    Female [[=cpp::refl::lowercase]] ,
    Male,
    Unknown 
};

template <> struct std::formatter<Gender> : cpp::enum_formatter<Gender> { };

struct [[=cpp::refl::pascal_case]] Student
{
    [[=cpp::refl::ignore]]
    std::string id;
    std::string name;
    int age;
    Gender gender;
    std::vector<int> scores;

    bool is_special;
};

template <> struct std::formatter<Student> : cpp::universal_formatter { };

int main() 
{
    Student alice{
        .id = "S001",
        .name = "Alice",
        .age = 20,
        .gender = Gender::Female,
        .scores = {90, 85, 92},
        .is_special = false
    };

    std::println("Student info: \n{}", alice);
}


