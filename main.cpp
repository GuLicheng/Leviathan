// #include <catch2/catch_all.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/annotations/all.hpp>
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
    "bValue": true,
    "UnnamedStruct": {
        "i32": 100,
        "f64": 3.14
    }
}

)";

enum struct 
[[=cpp::derive::decode<cpp::json::value>]] 
[[=cpp::derive::debug]]
Color
{
    Red [[=cpp::refl::rename("_RED_")]],
    Blue,
    Green
};

struct 
[[=cpp::derive::debug]]
[[=cpp::derive::decode<cpp::json::value>]] 
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
[[=cpp::derive::decode<cpp::json::value>]]
[[=cpp::refl::pascal_case]] 
[[=cpp::derive::debug]]
Derived : Base
{
    bool bValue;

    struct [[=cpp::derive::debug, =cpp::derive::decode<cpp::json::value>]] {
        int i32;
        double f64;
    } unnamed_struct; 
};

void TestAnnotation()
{
    auto j = cpp::json::loads(JsonString);
    auto d = cpp::cast<Derived>(j);

    std::println("{}", d.x);
    std::println("{}", d.NAME);
    std::println("{}", d.values);
    std::println("{}", d.color);
    std::println("{}", d.bValue);
    std::println("{}", d.unnamed_struct.i32);
    std::println("{}", d.unnamed_struct.f64);

    auto p = cpp::json::load(R"(D:\Library\Leviathan\test.json)");
    auto s = p["Name"].as<std::string>();
    std::println("{} - {}", s, s.size());
}

// TEST_CASE("Enum to string conversion with debug annotation", "[annotations]")
// {
//     auto j = cpp::json::loads(JsonString);
//     auto d = cpp::cast<Derived>(j);

//     REQUIRE(d.x == 42);
//     REQUIRE(d.NAME == "Test");
//     REQUIRE(d.values == std::vector{1.0, 2.0, 3.0});
//     REQUIRE(d.color == Color::Red);
//     REQUIRE(d.bValue == false);
//     // REQUIRE(d.unnamed_struct.i32 == 100);
//     // REQUIRE(d.unnamed_struct.f64 == 3.14);
// }

int main()
{
    TestAnnotation();
}