// This file only tests optional with reference.

#include <leviathan/config_parser/optional.hpp>
#include <catch2/catch_all.hpp>

using namespace leviathan::config;

TEST_CASE("ctor")
{
    int i = 0;

    optional<int&> op1;
    optional<int&> op2(nullopt);
    optional<int&> op3(i);
    optional<int&> op4(true, i);
    optional<int&> op5(false, i);
    optional<int&> op6(op3);
    

    REQUIRE(!op1);
    REQUIRE(!op2);
    REQUIRE(*op3 == 0);
    // REQUIRE(*op4 == 0);
    REQUIRE(!op5);
    // REQUIRE(*op6 == 0);
}

TEST_CASE("assign")
{
    int i = 0;
    optional<int&> op1;

    op1 = i;

    REQUIRE(*op1 == 0);

    op1 = nullopt;

    REQUIRE(!op1);
}

TEST_CASE("methods")
{
    int i = 0;
    optional<int&> op1;

    REQUIRE(!op1.has_value());

    op1 = i;

    REQUIRE(op1.has_value());
    REQUIRE(op1.value() == 0);

    op1.reset();
    REQUIRE(!op1.has_value());
}

