#include <print>
#include <iostream>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/annotations/all.hpp>
#include "leviathan/config_parser/json/json.hpp"
#include <string>
#include <algorithm>
#include <leviathan/extc++/format.hpp>
#include <ranges>
#include <numeric>
#include <array>

enum class
[[=cpp::derive::debug]]
[[=cpp::derive::hash]]
[[=cpp::derive::serialize<cpp::json::value>]]
[[=cpp::derive::deserialize<cpp::json::value>]]
Gender
{
    Female,
    Male,
    Unknown
};

struct 
[[=cpp::derive::debug]] 
[[=cpp::refl::pascal_case]]
[[=cpp::derive::hash]]
[[=cpp::derive::deserialize<cpp::json::value>]] 
[[=cpp::derive::serialize<cpp::json::value>]]
Student
{
    [[=cpp::refl::rename("userID")]]
    std::string id;
    std::string name;

    [[=cpp::refl::default_value(18.5)]]
    int age;

    [[=cpp::refl::choice(Gender::Female, Gender::Male)]]
    Gender gender;

    // std::vector<int> scores;

    [[=cpp::derive::skip]]
    bool is_special;

};

constexpr const char* context = R"(
    {
        "userID": "12345",
        "Name": "Alice",
        "is_student": true,
        "Gender": "Unknown",
        "scores": [85, 90, 92],
        "is_special": false
    }
)";



int main()
{
    auto root = cpp::json::loads(context);
    std::println("Parsed JSON: \n{:4}", root);
    auto student = cpp::cast<Student>(root);
    std::println("Student info: \n{}", student);
    std::println("Student hash: {}", std::hash<Student>()(student));


}


