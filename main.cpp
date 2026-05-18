#include <print>
#include <iostream>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/annotations/all.hpp>
#include <leviathan/extc++/enum.hpp>
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
[[=cpp::derive::decode<nlohmann::json>]]
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
[[=cpp::derive::decode<nlohmann::json>]]
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

template <typename T>
struct NlohmannJsonCaster;

template <typename T>
concept NlohmannJsonGetable = requires(const nlohmann::json& json) 
{
    { json.at("").get<T>() };
};

struct NlohmannJsonInitializer
{
    const nlohmann::json& json;

    NlohmannJsonInitializer(const nlohmann::json& json) : json(json) { }

    template <typename T>
    void operator()(std::optional<T>& value, std::string name) const
    {
        if (json.contains(name))
        {
            auto subobj = json.at(name);
            value.emplace(NlohmannJsonCaster<T>::operator()(subobj));
        }
    }
};

template <typename T>
struct NlohmannJsonCaster
{
    static constexpr T operator()(const nlohmann::json& json)
    {
        if constexpr (std::same_as<bool, T> || std::is_arithmetic_v<T> || std::same_as<std::string, T> || std::ranges::range<T>)
        {
            return json.get<T>();
        }
        else if constexpr (std::is_enum_v<T>)
        {
            auto str = json.get<std::string>();
            return cpp::enum_decoder<T>()(str);
        }
        else if constexpr (std::is_class_v<T>)
        {
            return cpp::refl::construct_struct<T>(NlohmannJsonInitializer(json));
        }
        else
        {
            static_assert(false, "No caster available for this type");
        }
    } 
};


int main()
{
    auto root = nlohmann::json::parse(context);
    std::cout << "Parsed JSON: \n" << root.dump(4) << std::endl;

    Student student = NlohmannJsonCaster<Student>::operator()(root);
    std::println("Student info: \n{}", student);
}


