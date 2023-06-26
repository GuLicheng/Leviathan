#include <leviathan/config_parser/json.hpp>
#include <catch2/catch_all.hpp>

#include <iostream>
#include <functional>

namespace json = leviathan::config::json;

TEST_CASE("json_make")
{
    json::json_value value1 = json::make(3);
    json::json_value value2 = json::make(true);
    json::json_value value3 = json::make("HelloWorld");
    json::json_value value4 = json::make(json::json_array());
    json::json_value value5 = json::make(json::json_object());
    json::json_value value6 = json::make(json::error_code::uninitialized);
    json::json_value value7 = json::make(json::json_null());

    REQUIRE(value1.is_number());
    REQUIRE(value1.is_integral());
    REQUIRE(value2.is_boolean());
    REQUIRE(value3.is_string());
    REQUIRE(value4.is_array());
    REQUIRE(value5.is_object());
    REQUIRE(!value6);
    REQUIRE(value7.is_null());

    REQUIRE(value1.as_number().value().as_signed_integral() == 3);
    REQUIRE(value2.as_boolean().value() == true);
    REQUIRE(value3.as_string().value() == "HelloWorld");
    REQUIRE(value4.as_array().value().empty());
    REQUIRE(value5.as_object().value().empty());
    // REQUIRE(value7.as_null().value() == json::json_null());
}

TEST_CASE("started with object and array")
{
    std::string context1[] = { R"({ "type": "object" })", R"([ "array" ])" };
    std::string context2[] = { R"(123)", R"("string")", R"(null)", R"(true)", R"(false)" };

    for (auto c : context1)
    {
        auto value = json::parser(std::move(c))();
        REQUIRE(value);
    }

    for (auto c : context2)
    {
        auto value = json::parser(std::move(c))();
        REQUIRE(value.ec() == json::error_code::illegal_root);
    }
}

TEST_CASE("error unicode")
{
    std::string error_unicodes[] = {
        R"(["\u"])",       // no digit
        R"(["\u621"])",    // less than 4 digits
        R"(["\uxyzw"])",   // not a hex-digit
        R"(["\u"])",
    };

    for (auto c : error_unicodes)
    {
        auto value = json::parser(std::move(c))();
        REQUIRE(value.ec() == json::error_code::illegal_unicode);
    }

}

// int main(int argc, char const *argv[])
// {
// 	system("chcp 65001");

//     json::json_value value1 = json::make(3);
//     json::json_value value2 = json::make(true);
//     json::json_value value3 = json::make("HelloWorld");
//     json::json_value value4 = json::make(json::json_array());
//     json::json_value value5 = json::make(json::json_object());
//     json::json_value value6 = json::make(json::error_code::uninitialized);

//     const char* path = R"(D:\Library\Leviathan\a.json)";

//     auto value = json::parse_json(path);

//     if (!value)
//     {
//         std::cout << json::report_error(*value.cast_unchecked<json::error_code>());
//     }
//     else
//     {
//         json::detail::json_serialize(std::cout, value, 0);
//     }

//     std::cout << "\nOk\n";

//     json::detail::json_serialize(std::cout, value["BORIS", "Cost"], 0);
//     json::detail::json_serialize(std::cout, value["BORIS", "Name"], 0);
//     json::detail::json_serialize(std::cout, value["BORIS", "Hero"], 0);

//     return 0;
// }

#if 0
class json_number
{
    enum struct type 
    {
        Integral,
        Floating,
    };

    union 
    {
        double m_f;
        ssize_t  m_i;
    };

    type m_type;

public:

    json_number() = delete;

    // Make it signed
    explicit json_number(std::signed_integral auto i) : m_i(i), m_type(type::Integral) { }

    explicit json_number(std::floating_point auto f) : m_f(f), m_type(type::Floating) { }

    bool is_integer() const
    { return m_type == type::Integral; }

    bool is_floating() const
    { return m_type == type::Floating; }

    explicit operator double() const
    {
        return is_integer() 
            ? static_cast<double>(m_i)
            : m_f;
    }

    explicit operator ssize_t() const
    {
        return is_integer()
            ? m_i
            : static_cast<ssize_t>(m_f);
    }
};
#endif