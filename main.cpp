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

enum class [[=cpp::derive::debug, =cpp::derive::hash]] Gender 
{
    Female,
    Male,
    Unknown 
};

struct [[=cpp::derive::debug, =cpp::refl::pascal_case, =cpp::derive::hash]] Student 
{
    std::string id;
    std::string name;
    [[=cpp::refl::default_value(18)]]
    int age;

    [[=cpp::refl::choice(Gender::Female, Gender::Male)]]
    Gender gender;
    // std::vector<int> scores;

    [[=cpp::derive::skip]]
    bool is_special;
};

template <>
inline constexpr bool cpp::use_default_caster<Student> = true;

constexpr const char* context = R"(
    {
        "id": "12345",
        "name": "Alice",
        "is_student": true,
        "gender": "Male",
        "scores": [85, 90, 92],
        "is_special": false
    }
)";

// template <std::meta::info Info, std::meta::info TemplateClass>
// consteval bool is_template_instance()
// {
//     bool is_template = std::meta::is_template(TemplateClass);

//     std::meta::has_template_arguments(Info, template_args);
// }

int main() 
{
    auto root = cpp::json::loads(context);
    std::println("Parsed JSON: \n{:4}", root);
    auto student = cpp::cast<Student>(root);
    std::println("Student info: \n{}", student);


    std::println("Student hash: {}", std::hash<Student>()(student));

    constexpr bool IsDerivedFromTemplate = std::is_base_of_v<std::meta::info, Student>;

}


