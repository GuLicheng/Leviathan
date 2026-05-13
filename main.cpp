#include <print>
#include <iostream>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/annotations/all.hpp>
#include "leviathan/config_parser/json/json.hpp"
#include <string>
#include <algorithm>
#include <leviathan/extc++/format.hpp>
#include <ranges>
#include <array>

enum class [[=cpp::derive::debug]] Gender 
{
    Female,
    Male,
    Unknown 
};

// template <> struct std::formatter<Gender> : cpp::enum_formatter { };

struct [[=cpp::derive::debug]] Student
{
    std::string id;
    std::string name;
    [[=cpp::refl::default_value(18)]]
    int age;
    Gender gender;
    std::vector<int> scores;

    [[=cpp::refl::ignore]]
    bool is_special;
};

// template <> struct std::formatter<Student> : cpp::universal_formatter { };

template <>
inline constexpr bool cpp::use_default_caster<Student> = true;

constexpr const char* context = R"(
    {
        "id": "12345",
        "name": "Alice",
        "is_student": true,
        "gender": "Female",
        "scores": [85, 90, 92],
        "is_special": false
    }
)";

struct [[=cpp::refl::ignore, =cpp::refl::ignore]] Foo { };

int main() 
{
    auto root = cpp::json::loads(context);
    std::println("Parsed JSON: \n{:4}", root);
    auto student = cpp::cast<Student>(root);
    std::println("Student info: \n{}", student);


    // constexpr auto N = cpp::refl::select_annotation(^^Foo, ^^cpp::refl::ignore_annotation).size();

}


