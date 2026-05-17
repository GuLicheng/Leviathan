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

struct [[=cpp::refl::parse_annotation]] BooleanParser
{
    static constexpr auto operator()(std::optional<bool>& out, std::string name)
    {
        auto lowercase = name | std::views::transform(::tolower) | std::ranges::to<std::string>();
        
        if (lowercase == "true") out.emplace(true);
        else if (lowercase == "false") out.emplace(false);
    }
};

struct [[=cpp::refl::value_annotation]] ReturnDefaultRange
{
    static constexpr std::initializer_list<int> operator()()
    {
        static auto ilist = std::initializer_list<int>{0, 1, 2};
        return ilist;
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

    [[=ReturnDefaultRange()]]
    std::vector<int> scores;

    bool is_special;

};

template <std::meta::info Info>
constexpr auto DefaultConstruct()
{
    using T = typename [:type_of(Info):];
    return T();
}

struct [[=cpp::derive::debug]] Foo
{
    int X;
    int Y;
    std::string Name;
};

struct JsonInitializer
{
    const cpp::json::value& root;    

    JsonInitializer(const cpp::json::value& root) : root(root) {}

    template <typename U>
    void operator()(std::optional<U>& opt, const std::string& name) const
    {
        assert(opt.has_value() == false);
    
        auto it = root.as<cpp::json::object>().find(cpp::json::string(name));

        if (it != root.as<cpp::json::object>().end())
        {
            opt.emplace(cpp::cast<U>(it->second));
        }
    }
};

constexpr const char* context = R"(
    {
        "userID": "12345",
        "Name": "Alice",
        "is_student": true,
        "Gender": "Unknown",
        "is_special": true
    }
)";

int main()
{
    auto root = cpp::json::loads(context);
    std::println("Parsed JSON: \n{:4}", root);
    auto student = cpp::cast<Student>(root);
    std::println("Student info: \n{}", student);

}


