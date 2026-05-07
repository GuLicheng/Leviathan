#include <leviathan/config_parser/json/json.hpp>
#include <iostream>
#include <meta>
#include <print>
#include "annotations.hpp"
#include <string>
#include <algorithm>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <ranges>
#include <array>

enum class Gender
{
    Male, Female
};

template <typename Enum, bool Enumerable = std::meta::is_enumerable_type(^^Enum)>
    requires std::is_enum_v<Enum>
constexpr Enum string_to_enum(std::string_view str)
{
    if constexpr (Enumerable)
        template for (constexpr auto e : std::define_static_array(enumerators_of(^^Enum)))
            if (str == identifier_of(e))
                return [:e:];
    throw std::runtime_error("Invalid enum string");
}

struct Student
{
    [[=cpp::refl::uppercase, =cpp::refl::lowercase]]
    int Age;

    [[=cpp::refl::lowercase, =cpp::refl::longname]]
    std::string Name;

    [[=cpp::refl::ignore]]
    Gender Gender;
    
    std::vector<int> Scores;
};

template <>
inline constexpr bool cpp::use_default_caster<Student> = true;

// void Test()
// {
//     cpp::json::value root = {
//         {"Age", 20},
//         {"Name", "Alice"},
//         {"Gender", "Female"},
//         {"Scores", {90, 85, 78}}
//     };

//     // auto student = parse<Student>(root);
//     auto student = cpp::cast<Student>(root);

//     std::print("Student: Age={}, Name={}, Gender={}, Scores={}\n", student.Age, student.Name, student.Gender, student.Scores);
// }

int main() 
{
    auto enum_str = "Female";
    auto gender = string_to_enum<Gender>(enum_str);
    std::print("String '{}' is converted to enum value '{}'\n", enum_str, std::to_underlying(gender));
}


