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
    json::json_value value8 = json::make(3ull);
    json::json_value value9 = json::make(3.14);

    REQUIRE(value1.is_number());
    REQUIRE(value1.is_integral());
    REQUIRE(value2.is_boolean());
    REQUIRE(value3.is_string());
    REQUIRE(value4.is_array());
    REQUIRE(value5.is_object());
    REQUIRE(!value6);
    REQUIRE(value7.is_null());
    REQUIRE(value8.is_number());
    REQUIRE(value9.is_number());

    REQUIRE(value1.as_number().value().as_signed_integral() == 3);
    REQUIRE(value2.as_boolean().value() == true);
    REQUIRE(value3.as_string().value() == "HelloWorld");
    REQUIRE(value4.as_array().value().empty());
    REQUIRE(value5.as_object().value().empty());
    // REQUIRE(value7.as_null().value() == json::json_null());
    REQUIRE(value8.as_number().value().is_unsigned_integral());
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
        REQUIRE(value.ec() == json::error_code::illegal_unicode);
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

    auto check = [](std::string s, int mode) {
        auto root = json::parser(std::move(s))();
        REQUIRE(root);
        switch (mode)
        {
            case 1: return root.as_number()->is_unsigned_integral();
            case 2: return root.as_number()->is_signed_integral();
            case 3: return root.as_number()->is_floating();
            default: return false;
        }
    };

    REQUIRE(check(numbers[0], 2));
    REQUIRE(check(numbers[1], 2));
    REQUIRE(check(numbers[2], 3));
    REQUIRE(check(numbers[3], 3));
    REQUIRE(check(numbers[4], 3));

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
    std::string s = R"""(
        {
            "empty string" : "",
            "simple string": "HelloWorld!",
            "Chinese": "\u6211\u7231\u5317\u4eac\u5929\u5b89\u95e8",
            "whitespace": "\r\t\n "
        }
    )""";

    auto root = json::parser(s)();

    REQUIRE(root.is_object());

    auto check_value = [&](const char* key, const char* target) {
        auto& value = root.as_object()->operator[](key);
        return value.as_string().value() == target;
    };

    REQUIRE(check_value("empty string", ""));
    REQUIRE(check_value("simple string", "HelloWorld!"));
    // REQUIRE(check_value("Chinese", "𝄞我爱北京天安门𝄞")); // GBK cannot response 𝄞, you can print it in screen.
    REQUIRE(check_value("whitespace", "\r\t\n "));
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
    const char* file = R"(D:\Library\Leviathan\leviathan\config_parser\config\annotation.json)";

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
        REQUIRE(object[name].as_number().value().as_signed_integral() == value);
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
