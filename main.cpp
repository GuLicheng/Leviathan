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
    
    Female [[=cpp::refl::lowercase, =cpp::refl::ignore]],

    Unknown [[=cpp::refl::rename("unknown_gender")]],
};

template <typename Enum>
struct EnumEncoder
{
    static_assert(std::is_enum_v<Enum>, "universal_enum_parser can only be used with enum types");

    static constexpr std::string operator()(Enum value)
    {
        template for (constexpr auto e : define_static_array(enumerators_of(^^Enum)))
        {
            if (value == [:e:])
            {            
                auto name = std::string(identifier_of(e));
                
                template for (constexpr auto anno : define_static_array(annotations_of(e)))
                {
                    using AnnoType = typename [:type_of(anno):];

                    if constexpr (std::is_base_of_v<cpp::refl::rename_annotation, AnnoType>)
                    {
                        name = std::invoke(extract<AnnoType>(anno), name);
                    }
                    else if constexpr (std::is_base_of_v<cpp::refl::ignore_annotation, AnnoType>)
                    {
                        return "<ignored>";
                    }
                }
                return name;
            }
        }
        return "<unnamed>";
    }
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

    std::println("{}", EnumEncoder<Gender>::operator()(Gender::Male));
    std::println("{}", EnumEncoder<Gender>::operator()(Gender::Female));
    std::println("{}", EnumEncoder<Gender>::operator()(Gender::Unknown));
}


