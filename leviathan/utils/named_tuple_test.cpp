#include "named_tuple.hpp"
#include <catch2/catch_all.hpp>

TEST_CASE("simple test")
{
    using NameField = cpp::field<"name", std::string>;
    using AgeField = cpp::field<"age", int>;

    using Struct = cpp::named_tuple<
        NameField,
        AgeField
    >;

    Struct s("name"_arg = std::string("Alice"), "age"_arg = 5);

    REQUIRE(s["name"_arg] == "Alice");
    REQUIRE(s["age"_arg] == 5);
}

TEST_CASE("type convert")
{
    using NameField = cpp::field<"name", std::string>;
    using AgeField = cpp::field<"age", std::optional<int>>;
    using OtherField = cpp::field<"other", std::optional<std::string>>;

    using Struct = cpp::named_tuple<
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
    using NameField = cpp::field<"name", std::string>;
    using AgeField = cpp::field<"age", int>;

    using Struct = cpp::named_tuple<
        NameField,
        AgeField
    >;

    Struct s("name"_arg = "Alice", "age"_arg = 5);

    auto&& [name, age] = s;
    REQUIRE(name == "Alice");
    REQUIRE(age == 5);
}

TEST_CASE("reorder automatically")
{
    using NameField = cpp::field<"name", std::string>;
    using AgeField = cpp::field<"age", int>;

    using Struct = cpp::named_tuple<
        NameField,
        AgeField
    >;

    Struct s("age"_arg = 5, "name"_arg = "Alice");

    REQUIRE(s["name"_arg] == "Alice");
    REQUIRE(s["age"_arg] == 5);
}

TEST_CASE("move test")
{
    struct MoveOnly 
    {
        MoveOnly() = default;
        MoveOnly(MoveOnly&&) = default;
        MoveOnly(const MoveOnly&) = delete;
    };

    using Field1 = cpp::field<"moveonly", MoveOnly>;
    using Field2 = cpp::field<"int", int>;

    using MoveOnlyStruct = cpp::named_tuple<Field1, Field2>;

    // Adjust arguments
    MoveOnlyStruct s("int"_arg = 1, "moveonly"_arg = MoveOnly());

    REQUIRE(s["int"_arg] == 1);
}

