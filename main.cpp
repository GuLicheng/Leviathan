#include <print>
#include <iostream>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/annotations/all.hpp>
#include "leviathan/config_parser/json/json.hpp"
#include <string>
#include <algorithm>
#include <leviathan/extc++/format.hpp>
#include "nlohmann.hpp"
#include <ranges>
#include <numeric>
#include <array>
#include <variant>

enum class
[[=cpp::derive::debug]]
[[=cpp::derive::hash]]
[[=cpp::derive::encode<cpp::json::value>]]
[[=cpp::derive::decode<cpp::json::value>]]
[[=cpp::derive::op_pipe]]
Gender
{
    Female,
    Male,
    Unknown
};

struct [[=cpp::refl::choice_annotation]] GreatThan
{
    int x;

    consteval GreatThan(int x) : x(x) { }

    constexpr bool operator()(const auto& input) const
    {
        return input > x;
    }
};

struct 
[[=cpp::derive::debug]] 
[[=cpp::refl::pascal_case]]
[[=cpp::derive::decode<cpp::json::value>]] 
[[=cpp::derive::encode<cpp::json::value>]]
Student
{
    [[=cpp::refl::rename("userID")]]
    std::string id;

    std::string name;

    [[=cpp::refl::default_value(18.5), =cpp::refl::range(15, 150), =GreatThan(0)]]
    int age;

    [[=cpp::refl::choice(Gender::Female, Gender::Male, Gender::Unknown)]]
    Gender gender;

    std::vector<int> scores;

    bool is_special;

};

constexpr const char* context = R"(
    {
        "userID": "12345",
        "Name": "Alice",
        "is_student": true,
        "Gender": "Unknown",
        "is_special": true,
        "scores": [85, 90, 78]
    }
)";

void TestJson()
{
    auto root = cpp::json::loads(context);
    std::println("Parsed JSON: \n{:4}", root);
    auto student = cpp::cast<Student>(root);
    std::println("Student info: \n{}", student);
}

struct NlohmannJsonInitializer
{
    nlohmann::json json;

    NlohmannJsonInitializer(nlohmann::json json) : json(std::move(json)) { }

    template <typename T>
    void operator()(std::optional<T>& value, std::string name) const
    {
        value.emplace(SimpleFromJson<T>(json));
    }

    template <typename T>
    T SimpleToStruct(std::optional<T>& value, std::string name);

    template <typename E>
    E SimpleToEnum(std::optional<E>& value, std::string name);

    
};

int main()
{
    auto root = nlohmann::json::parse(context);
    std::cout << "Parsed JSON: \n" << root.dump(4) << std::endl;
}


