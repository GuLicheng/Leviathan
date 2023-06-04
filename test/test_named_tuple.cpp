#include <leviathan/named_tuple.hpp>
#include <catch2/catch_all.hpp>

TEST_CASE("simple test")
{
    using NameField = leviathan::field<"name", std::string>;
    using AgeField = leviathan::field<"age", int>;

    using Struct = leviathan::named_tuple<
        NameField,
        AgeField
    >;

    Struct s("name"_arg = std::string("Alice"), "age"_arg = 5);

    REQUIRE(s["name"_arg] == "Alice");
    REQUIRE(s["age"_arg] == 5);
}

TEST_CASE("type convert")
{
    using NameField = leviathan::field<"name", std::string>;
    using AgeField = leviathan::field<"age", std::optional<int>>;
    using OtherField = leviathan::field<"other", std::optional<std::string>>;

    using Struct = leviathan::named_tuple<
        NameField,
        AgeField,
        OtherField
    >;

    Struct s("name"_arg = "Alice", "age"_arg = 5);

    REQUIRE(s["name"_arg] == "Alice");
    REQUIRE(s["age"_arg] == 5);
    REQUIRE(!s["other"_arg].has_value());
}

TEST_CASE("structured binding")
{
    using NameField = leviathan::field<"name", std::string>;
    using AgeField = leviathan::field<"age", int>;

    using Struct = leviathan::named_tuple<
        NameField,
        AgeField
    >;

    Struct s("name"_arg = "Alice", "age"_arg = 5);

    auto&& [name, age] = s;
    REQUIRE(name == "Alice");
    REQUIRE(age == 5);
}
