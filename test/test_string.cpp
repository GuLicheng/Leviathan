#include <leviathan/string/lexical_cast.hpp>
#include <leviathan/string/fixed_string.hpp>
#include <leviathan/string/string_extend.hpp>
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

TEST_CASE("lexical cast from self to self")
{
    std::string_view s = "12345";

    auto i = leviathan::string::lexical_cast<std::string_view>(s);

    REQUIRE(i == "12345");
}

TEST_CASE("ltrim")
{
    std::string_view sv1 = " HelloWorld";
    std::string_view sv2 = "";
    std::string_view sv3 = "        ";
    std::string_view sv4 = "  HelloWorld   ";

    using leviathan::string::ltrim;

    REQUIRE(ltrim(sv1) == "HelloWorld");
    REQUIRE(ltrim(sv2) == "");
    REQUIRE(ltrim(sv3) == "");
    REQUIRE(ltrim(sv4) == "HelloWorld   ");
}

TEST_CASE("rtrim")
{
    std::string_view sv1 = "HelloWorld ";
    std::string_view sv2 = "";
    std::string_view sv3 = "        ";
    std::string_view sv4 = "  HelloWorld   ";

    using leviathan::string::rtrim;

    REQUIRE(rtrim(sv1) == "HelloWorld");
    REQUIRE(rtrim(sv2) == "");
    REQUIRE(rtrim(sv3) == "");
    REQUIRE(rtrim(sv4) == "  HelloWorld");
}

TEST_CASE("trim1")
{
    std::string_view sv1 = "HelloWorld ";
    std::string_view sv2 = "";
    std::string_view sv3 = "        ";
    std::string_view sv4 = "  HelloWorld   ";

    using leviathan::string::trim;

    REQUIRE(trim(sv1) == "HelloWorld");
    REQUIRE(trim(sv2) == "");
    REQUIRE(trim(sv3) == "");
    REQUIRE(trim(sv4) == "HelloWorld");
}

TEST_CASE("trim2")
{
{
    constexpr std::string_view answer1 = "hello world!";
    constexpr std::string_view answer2 = "";

    constexpr std::string_view sv0 = "   hello world!";
    constexpr std::string_view sv1 = " \t\n\t   ";
    constexpr std::string_view sv2 = "hello world!   ";
    constexpr std::string_view sv3 = "hello world!";

    using leviathan::string::trim;

    REQUIRE(trim(sv0) == answer1);
    REQUIRE(trim(sv1) == answer2);
    REQUIRE(trim(sv2) == answer1);
    REQUIRE(trim(sv3) == answer1);
}
}
