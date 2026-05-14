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

    [[=cpp::refl::default_value(18)]]
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
        "gender": "Unknown",
        "scores": [85, 90, 92],
        "is_special": false
    }
)";

template <std::meta::info ClassInfo, std::meta::info FieldInfo>
struct field_initializer
{
    static_assert(std::meta::is_class_type(ClassInfo) && std::meta::is_class_member(FieldInfo));

    using FieldType = typename [:type_of(FieldInfo):];

    template <typename Initializer>
    static constexpr FieldType operator()(Initializer initializer)
    {
        // Get field name
        auto name = cpp::refl::extract_name_by_annotation<FieldInfo, ClassInfo>();

        std::optional<FieldType> value = std::nullopt;
        initializer(value, name);

        if (!value)
        {
            template for (constexpr auto anno : define_static_array(annotations_of(FieldInfo)))
            {
                using AnnoType = typename [:type_of(anno):];

                if constexpr (std::is_base_of_v<cpp::refl::value_annotation<FieldType>, AnnoType>)
                {
                    value.emplace(std::invoke(extract<AnnoType>(anno)));
                    break;  // Only the first value annotation is effective
                }
            }

            if (!value)
            {
                value.emplace(); // default initialize
            }
        }

        // Check the value is legal

        template for (constexpr auto anno : define_static_array(annotations_of(FieldInfo)))
        {

        }

        return value ? std::move(*value) : throw std::runtime_error(std::format("Field {} is missing or invalid", name));
    }

};


int main()
{
    auto root = cpp::json::loads(context);
    std::println("Parsed JSON: \n{:4}", root);
    auto student = cpp::cast<Student>(root);
    std::println("Student info: \n{}", student);
    std::println("Student hash: {}", std::hash<Student>()(student));

    // auto tuple = struct_to_tuple(student);
    // std::println("Student as tuple: {}", tuple);

    std::index_sequence<0, 1, 2, 3> seq;

    // auto [a, b, c, d] = seq;
    // auto [...xs] = std::make_tuple(1, 2, 3);
}


