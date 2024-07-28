#include "json.hpp"
#include <catch2/catch_all.hpp>

#include <numeric>
#include <iostream>
#include <functional>

namespace json = leviathan::json;

// using json::null;
// using json::number;
// using json::boolean;
// using json::array;
// using json::object;
// using json::value;
// using json::string;
using json::error_code;
using json::make_json;

TEST_CASE("json_make")
{
    json::value value1 = make_json<json::number>(3);
    json::value value2 = make_json<json::boolean>(true);
    json::value value3 = make_json<json::string>("HelloWorld");
    json::value value4 = make_json<json::array>();
    json::value value5 = make_json<json::object>();
    json::value value6 = make_json<error_code>(error_code::uninitialized);
    json::value value7 = make_json<json::null>(nullptr);
    json::value value8 = make_json<json::number>(3ull);
    json::value value9 = make_json<json::number>(3.14);

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

    REQUIRE(value1.as<json::number>().as_signed_integer() == 3);
    REQUIRE(value2.as<json::boolean>() == true);
    REQUIRE(value3.as<json::string>() == "HelloWorld");
    REQUIRE(value4.as<json::array>().empty());
    REQUIRE(value5.as<json::object>().empty());
    REQUIRE(value7.as<json::null>() == json::null());
    REQUIRE(value8.as<json::number>().is_unsigned_integer());
    REQUIRE(value9.as<json::number>().is_floating());
}

TEST_CASE("number")
{
    std::string numbers[] = {
        R"([123])",     // integer
        R"([-1])",      // integer
        R"([3.14])",    // double
        R"([2.7e18])",  // double
        R"([-0.1])",    // double
    };

    auto check = []<typename T>(std::string s, T expected) {
        auto root = json::loads(std::move(s));
        REQUIRE(root.is_array());
        return root.as<json::array>()[0].as<json::number>() == json::number(expected);
    };

    REQUIRE(check(numbers[0], 123));
    REQUIRE(check(numbers[1], -1));
    REQUIRE(check(numbers[2], 3.14));
    REQUIRE(check(numbers[3], 2.7e18));
    REQUIRE(check(numbers[4], -0.1));

    REQUIRE(check("[2147483647]", 2147483647));
    REQUIRE(check("[-2147483648]", -2147483648));
    REQUIRE(check("[4294967295]", 4294967295));
    REQUIRE(check("[18446744073709551615]", std::size_t(-1)));
    REQUIRE(check("[0]", 0));
    REQUIRE(check("[-0]", 0));
    REQUIRE(check("[1]", 1));
    REQUIRE(check("[1.2345678]", 1.2345678));
    REQUIRE(check("[0.12345678e7]", 1234567.8));
    REQUIRE(check("[19000000000000000001]", 1.9e+19));
    REQUIRE(check("[-9300000000000000001]", -9.3e+18));

    std::string error_numbers[] = {
        R"([2.7e18e])",
        R"([2..7e18e])",
        R"([2e.7e18e])",
    };

    for (auto c : error_numbers)
    {
        auto value = json::loads(std::move(c));
        REQUIRE(value.ec() == json::error_code::illegal_number);
    }
}

TEST_CASE("string")
{
    SECTION("valid")
    {
        auto check_value = [&](const char* key, const char* target) {
            std::string s = key;
            auto list = json::loads(std::format("[{}]", s));
            REQUIRE(list);
            auto& root = list.as<json::array>()[0];
            return root.is_string() && root.as<json::string>() == target;
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
            "['//this is bad JSON.']",
        };

        for (auto c : strs)
        {
            auto value = json::loads(std::move(c));
            REQUIRE(!value);
        }
    }
}

TEST_CASE("literal")
{
    std::string s = R"""(
        [true, false, null]
    )""";

    auto value = json::loads(s);

    REQUIRE(value.is_array());
    REQUIRE(value.as<json::array>().at(0).as<json::boolean>() == true);
    REQUIRE(value.as<json::array>().at(1).as<json::boolean>() == false);
    REQUIRE(value.as<json::array>().at(2).is_null());
}

TEST_CASE("annotation")
{
    const char* file = R"(D:\Library\Leviathan\leviathan\config_parser\data\json\annotation.json)";

    auto root = json::load(file);

    REQUIRE(root);

    REQUIRE(root.is_object());

    auto& object = root.as<json::object>()["00000_FV.json"].as<json::object>();

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
        REQUIRE(object[name].as<json::number>().as_signed_integer() == value);
    };

    check_number("image_channels", 3);
    check_number("image_height", 966);
    check_number("image_width", 1280);
    check_number("job_batch_id", 34314);
    check_number("job_id", 212425942);

    REQUIRE(object["status"].is_string());
    REQUIRE(object["status"].as<json::string>() == "finished");

    REQUIRE(object["annotation-tags"].is_array());
    auto& annotags = object["annotation-tags"].as<json::array>();

    std::ranges::sort(annotags, {}, [](json::value& value) {
        return value.as<json::string>();
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
        REQUIRE(annotags[i].as<json::string>() == tags[i]);
    }

    REQUIRE(object["annotation"].is_array());
    auto& anno = object["annotation"].as<json::array>();

    int id = 0;

    for (auto& val : anno)
    {
        if (val.is_object())
        {
            id += val.as<json::object>().count("id");
        }
    }

    REQUIRE(id == 106);
}

TEST_CASE("multi-dim operator[]")
{
    auto obj = json::make_json<json::object>();

    obj["Hello", "World"];

}

TEST_CASE("failed cases")
{
    auto check = [](std::string s) {
        auto root = json::loads(s);
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

TEST_CASE("files")
{
    std::string paths[] = {
        R"(D:\Library\Leviathan\leviathan\config_parser\data\json\passa1.json)",
        R"(D:\Library\Leviathan\leviathan\config_parser\data\json\twitter.json)",
        R"(D:\Library\Leviathan\leviathan\config_parser\data\json\citm_catalog.json)",
    };

    for (const auto& path : paths)
    {
        auto root = json::load(path.c_str());
        REQUIRE(root);
    }
}


