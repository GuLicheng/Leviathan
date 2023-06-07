#include <leviathan/builder.hpp>
#include <catch2/catch_all.hpp>

TEST_CASE("build with name and index")
{
    struct Foo
    {
        std::string name;
        int age;
    };

    auto res = leviathan::builder<"name", "age">::with<std::string, int>()
        .build_member<"name">("Alice")
        .build_member<1>(18)
        .build<Foo>();

    REQUIRE(res->name == "Alice");
    REQUIRE(res->age == 18);
}

