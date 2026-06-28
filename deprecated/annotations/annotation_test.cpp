#include <catch2/catch_all.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/enum.hpp>
#include <print>

constexpr const char* JsonString = R"(

{
    "X": 42,
    "name": "Test",
    "values": [1.0, 2.0, 3.0],
    "color": "_RED_",
    "BooleanValue": true,
    "UnnamedStruct": {
        "i32": 100,
        "f64": 3.14
    }
}

)";

enum struct 
[[=cpp::derive::from<cpp::json::value>]] 
[[=cpp::derive::debug]]
Color
{
    Red [[=cpp::refl::rename("_RED_")]],
    Blue,
    Green
};

struct 
[[=cpp::derive::debug]]
[[=cpp::derive::from<cpp::json::value>]] 
Base
{
    [[=cpp::refl::uppercase]]
    int x;

    [[=cpp::refl::lowercase]]
    std::string NAME;

    std::vector<double> values;

    Color color;
};

struct 
[[=cpp::derive::from<cpp::json::value>]]
[[=cpp::refl::pascal_case]] 
[[=cpp::derive::debug]]
Derived : Base
{
    bool BooleanValue;  

    struct [[=cpp::derive::debug, =cpp::derive::from<cpp::json::value>]] {
        int i32;
        double f64;
    } unnamed_struct; 
};

TEST_CASE("Enum to string conversion with debug annotation", "[annotations]")
{
    auto j = cpp::json::loads(JsonString);
    auto d = cpp::cast<Derived>(j);

    REQUIRE(d.x == 42);
    REQUIRE(d.NAME == "Test");
    REQUIRE(d.values == std::vector{1.0, 2.0, 3.0});
    REQUIRE(d.color == Color::Red);
    REQUIRE(d.BooleanValue == true);
    REQUIRE(d.unnamed_struct.i32 == 100);
    REQUIRE(d.unnamed_struct.f64 == 3.14);
}

