#include <leviathan/string/lexical_cast.hpp>
#include <catch2/catch_all.hpp>

TEST_CASE("lexical cast from std::string to integer")
{
    std::string s = "-1";

    auto i = leviathan::string::lexical_cast<int>(s);

    REQUIRE(i.has_value());
    REQUIRE(*i == -1);
}

TEST_CASE("lexical cast from std::string to unsigned integer")
{
    std::string s = "12345";

    auto i = leviathan::string::lexical_cast<unsigned int>(s);

    REQUIRE(i.has_value());
    REQUIRE(*i == 12345);
}

TEST_CASE("lexical cast from std::string to floating")
{
    std::string s = "3.14";

    auto i = leviathan::string::lexical_cast<double>(s);

    REQUIRE(i.has_value());

    auto val = std::abs(*i - 3.14);
    REQUIRE(val < 1e-5);
}

TEST_CASE("lexical cast from std::string_view to unsigned integer")
{
    std::string_view s = "12345";

    auto i = leviathan::string::lexical_cast<unsigned int>(s);

    REQUIRE(i.has_value());
    REQUIRE(*i == 12345);
}

TEST_CASE("lexical cast from leviathan::fixed_string to unsigned integer")
{
    leviathan::fixed_string s = "12345";

    auto i = leviathan::string::lexical_cast<unsigned int>(s);

    REQUIRE(i.has_value());
    REQUIRE(*i == 12345);
}
