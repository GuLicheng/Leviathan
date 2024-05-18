#include <leviathan/config_parser/json.hpp>
#include <catch2/catch_all.hpp>

#include <numeric>
#include <iostream>
#include <functional>

namespace json = leviathan::json;

TEST_CASE("json_make")
{
    json::json_value value1 = json::make(3);
    json::json_value value2 = json::make(true);
    json::json_value value3 = json::make("HelloWorld");
    json::json_value value4 = json::make(json::json_array());
    json::json_value value5 = json::make(json::json_object());
    json::json_value value6 = json::make(json::error_code::uninitialized);
    json::json_value value7 = json::make(json::json_null());
    json::json_value value8 = json::make(3ull);
    json::json_value value9 = json::make(3.14);

    REQUIRE(value1.is_number());
    REQUIRE(value1.is_integer());
    REQUIRE(value2.is_boolean());
    REQUIRE(value3.is_string());
    REQUIRE(value4.is_array());
    REQUIRE(value5.is_object());
    REQUIRE(!value6);
    REQUIRE(value7.is_null());
    REQUIRE(value8.is_number());
    REQUIRE(value9.is_number());

    REQUIRE(value1.as_number().value().as_signed_integer() == 3);
    REQUIRE(value2.as_boolean().value() == true);
    REQUIRE(value3.as_string().value() == "HelloWorld");
    REQUIRE(value4.as_array().value().empty());
    REQUIRE(value5.as_object().value().empty());
    REQUIRE(value7.as_null().value() == json::json_null());
    REQUIRE(value8.as_number().value().is_unsigned_integer());
    REQUIRE(value9.as_number().value().is_floating());
}

TEST_CASE("unicode")
{
    std::string error_unicodes[] = {
        R"("\u")",       // no digit
        R"("\u621")",    // less than 4 digits
        R"("\uxyzw")",   // not a hex-digit
    };

    for (auto c : error_unicodes)
    {
        auto value = json::parser(std::move(c))();
        REQUIRE(!value);
    }
}

TEST_CASE("number")
{
    std::string numbers[] = {
        R"(123)",     // integer
        R"(-1)",      // integer
        R"(3.14)",    // double
        R"(2.7e18)",  // double
        R"(-0.1)",    // double
    };

    auto check = []<typename T>(std::string s, T expected) {
        auto root = json::parser(std::move(s))();
        REQUIRE(root);

        if constexpr (std::signed_integral<T>)
        {
            return root.as_number()->is_signed_integer() 
                && root.as_number()->as_signed_integer() == expected;
        }
        else if constexpr (std::unsigned_integral<T>)
        {
            return root.as_number()->is_unsigned_integer() 
                && root.as_number()->as_unsigned_integer() == expected;
        }
        else if constexpr (std::floating_point<T>)
        {
            static_assert(std::is_same_v<double, T>);
            return root.as_number()->is_floating()
                && (std::abs(root.as_number()->as_floating() - expected) < 1e-7); 
        }
        else
        {
            return false;
        }
    };

    REQUIRE(check(numbers[0], 123));
    REQUIRE(check(numbers[1], -1));
    REQUIRE(check(numbers[2], 3.14));
    REQUIRE(check(numbers[3], 2.7e18));
    REQUIRE(check(numbers[4], -0.1));

    REQUIRE(check("2147483647", 2147483647));
    REQUIRE(check("-2147483648", -2147483648));
    REQUIRE(check("4294967295", 4294967295));
    REQUIRE(check("18446744073709551615", std::size_t(-1)));
    REQUIRE(check("0", 0));
    REQUIRE(check("-0", 0));
    REQUIRE(check("1", 1));
    REQUIRE(check("1.2345678", 1.2345678));
    REQUIRE(check("0.12345678e7", 1234567.8));
    REQUIRE(check("19000000000000000001", 1.9e+19));
    REQUIRE(check("-9300000000000000001", -9.3e+18));
    // REQUIRE(check("1e+9999", std::numeric_limits<double>::infinity()));
    // REQUIRE(check("-1e+9999", -std::numeric_limits<double>::infinity()));

    std::string error_numbers[] = {
        R"(2.7e18e)",
        R"(2..7e18e)",
        R"(2e.7e18e)",
    };

    for (auto c : error_numbers)
    {
        auto value = json::parser(std::move(c))();
        REQUIRE(value.ec() == json::error_code::illegal_number);
    }
}

TEST_CASE("string")
{
    SECTION("valid")
    {
        auto check_value = [&](const char* key, const char* target) {
            std::string s = key;
            auto root = json::load(s);
            REQUIRE(root);
            return root.is_string() && root.as_string().value() == target;
        };

        auto src1 = R"("!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")";
        auto src2 = R"(!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)";
        REQUIRE(check_value(src1, src2));
        REQUIRE(check_value(R"("http:\/\/jsoncpp.sourceforge.net\/")", "http://jsoncpp.sourceforge.net/"));
        REQUIRE(check_value(R"("\"abc\\def\"")", R"("abc\def")"));
        REQUIRE(check_value(R"("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\")", R"(\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\)"));
        REQUIRE(check_value(R"("\u0061")", "a"));
        REQUIRE(check_value(R"("\u00A2")", "¢"));
        REQUIRE(check_value(R"("\u20AC")", "€"));
        REQUIRE(check_value(R"("\uD834\uDD1E")", "𝄞"));
        REQUIRE(check_value(R"("Zażółć gęślą jaźń")", "Zażółć gęślą jaźń"));
    }

    SECTION("invalid")
    {
        std::string strs[] = {
            R"('//this is bad JSON.')",
        };

        for (auto c : strs)
        {
            auto value = json::parser(std::move(c))();
            REQUIRE(!value);
        }
    }
}

TEST_CASE("literal")
{
    std::string s = R"""(
        [true, false, null]
    )""";

    auto value = json::parser(s)();

    REQUIRE(value.is_array());
    REQUIRE(value.as_array()->at(0).as_boolean().value() == true);
    REQUIRE(value.as_array()->at(1).as_boolean().value() == false);
    REQUIRE(value.as_array()->at(2).is_null());
}

TEST_CASE("annotation")
{
    const char* file = R"(D:\Library\Leviathan\leviathan\config_parser\data\json\annotation.json)";

    auto root = json::parse_json(file);

    REQUIRE(root);

    REQUIRE(root.is_object());

    auto& object = root.as_object().value()["00000_FV.json"].as_object().value();

    std::string keys[] = {
        "annotation",
        "annotation-tags",
        "feedback",
        "image_channels",
        "image_height",
        "image_width",
        "job_batch_id",
        "status",  
    };

    for (const auto& key : keys)
    {
        REQUIRE(object.contains(key));
    }

    REQUIRE(object["feedback"].is_null());

    auto check_number = [&](const char* name, int value) {
        REQUIRE(object[name].is_number());
        REQUIRE(object[name].as_number().value().as_signed_integer() == value);
    };

    check_number("image_channels", 3);
    check_number("image_height", 966);
    check_number("image_width", 1280);
    check_number("job_batch_id", 34314);
    check_number("job_id", 212425942);

    REQUIRE(object["status"].is_string());
    REQUIRE(object["status"].as_string().value() == "finished");

    REQUIRE(object["annotation-tags"].is_array());
    auto& annotags = object["annotation-tags"].as_array().value();

    std::ranges::sort(annotags, {}, [](auto& value) {
        return value.as_string().value();
    });

    std::string tags[] = {
        "green_strip",
        "ego_vehicle",
        "bus",
        "car",
        "movable_object",
        "person",
        "construction",
        "grouped_pedestrian_and_animals",
        "traffic_sign",
        "unknown_traffic_light",
        "traffic_light_red",
        "pole",
        "construction",
        "lane_marking",
        "road_surface",
        "curb",
        "free_space",
        "sky",
        "nature",
        "fence"
    };

    std::ranges::sort(tags);

    REQUIRE(annotags.size() == std::ranges::size(tags));

    for (size_t i = 0; i < annotags.size(); ++i)
    {
        REQUIRE(annotags[i].as_string().value() == tags[i]);
    }

    REQUIRE(object["annotation"].is_array());
    auto& anno = object["annotation"].as_array().value();

    int id = 0;

    for (auto& val : anno)
    {
        if (val.is_object())
        {
            id += val.as_object().value().count("id");
        }
    }

    REQUIRE(id == 106);
}

TEST_CASE("multi-dim operator[]")
{
    auto obj = json::make(json::json_object());

    obj["Hello", "World"];

}

TEST_CASE("failed cases")
{
    auto check = [](std::string s) {
        auto root = json::load(s);
        REQUIRE(!root);
    };

    // check(R"("A JSON payload should be an object or array, not a string.")");
    check(R"(["Unclosed array")");
    check(R"({unquoted_key: "keys must be quoted"})");
    check(R"(["extra comma",])");
    check(R"(["double extra comma",,])");
    check(R"([   , "<-- missing value"])");
    check(R"(["Comma after the close"],)");
    check(R"(["Extra close"]])");
    check(R"({"Extra comma": true,})");
    check(R"(["Extra close"]])");
    check(R"({"Extra value after close": true} "misplaced quoted value")");
    check(R"({"Illegal expression": 1 + 2})");
    check(R"({"Illegal invocation": alert()})");
    check(R"({"Numbers cannot have leading zeroes": 013})");
    check(R"({"Numbers cannot be hex": 0x14})");
    check(R"(["Illegal backslash escape: \x15"])");
    check(R"([\naked])");
    check(R"(["Illegal backslash escape: \017"])");
    // check(R"([[[[[[[[[[[[[[[[[[[["Too deep"]]]]]]]]]]]]]]]]]]]])");
    check(R"({"Missing colon" null})");
    check(R"({"Double colon":: null})");
    check(R"({"Comma instead of colon", null})");       
    check(R"(["Colon instead of comma": false])");
    check(R"(["Bad value", truth])");
    check(R"(['single quote'])");
    check(R"([0e])");
    check(R"([0e+])");
    check(R"([0e+-1])");
    check(R"({"Comma instead if closing brace": true,)");
    check(R"(["mismatch"})");
}

TEST_CASE("pass1")
{
    const char* path = R"(D:\Library\Leviathan\leviathan\config_parser\data\json\passa1.json)";

    auto root = json::parse_json(path);

    REQUIRE(root);
}



// int main(int argc, char const *argv[])
// {
//     system("chcp 65001");

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
