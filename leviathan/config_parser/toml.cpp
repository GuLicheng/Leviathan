#include <leviathan/config_parser/toml.hpp>
#include <catch2/catch_all.hpp>

namespace toml = leviathan::toml;

TEST_CASE("test split key")
{
    auto check = [](std::string_view key, auto... svs) {
        
        auto idx = key.find('=');

        REQUIRE(idx != key.npos);

        key = key.substr(0, idx);

        auto res = toml::detail::split_keys(key);

        std::string_view keys[] = { svs... };

        REQUIRE(std::ranges::equal(res, keys));
    };

    SECTION("valid input")
    {
        check(R"(name = "Orange")", "name");
        check(R"(physical.color = "orange")", "physical", "color");
        check(R"(site."google.com" = true)", "site", "google.com");
        check(R"("" = "blank")", "");
        check(R"('' = "blank")", "");
        check(R"("127.0.0.1" = "value")", "127.0.0.1");
        check(R"("character encoding" = "value")", "character encoding");
        check(R"("ʎǝʞ" = "value")", "ʎǝʞ");
        check(R"('key2' = "value")", "key2");
        check(R"('quoted "value"' = "value")", "quoted \"value\"");
        check(R"(key = "value"")", "key");
        check(R"('key2' = "value")", "key2");
        check(R"('bare_key' = "value")", "bare_key");
        check(R"('bare-key' = "value")", "bare-key");
        check(R"(1234 = "value")", "1234");
        check(R"(fruit.name = "value")", "fruit", "name");
        check(R"(fruit. name = "value")", "fruit", "name");
        check(R"(fruit . name = "value")", "fruit", "name");
    }

    SECTION("invalid input")
    {
        // std::string_view s[] = {
        //     R"(= "no key name")", 
        // };

        // REQUIRE_THROWS()
    }
}

TEST_CASE("comment")
{
    std::string context = R"(
        # Global
        key = true # comment
    )";

    auto root = toml::load(context);
    REQUIRE(root.as_table().find("key")->second.as_boolean() == true);
}

TEST_CASE("boolean")
{
    std::string context = R"(
        true=true
        false=false
    )";

    auto root = toml::load(context);
    REQUIRE(root.as_table().find("true")->second.as_boolean() == true);
    REQUIRE(root.as_table().find("false")->second.as_boolean() == false);
}

TEST_CASE("integer")
{
    SECTION("valid")
    {
        std::string context = R"(
            int1 = +99
            int2 = 42
            int3 = 0
            int4 = -17
            int5 = 1_000
            int6 = 5_349_221
            int7 = 53_49_221
            int8 = 1_2_3_4_5
            
            hex1 = 0xDEADBEEF
            hex2 = 0xdeadbeef
            hex3 = 0xdead_beef
            
            oct1 = 0o01234567
            oct2 = 0o755

            bin1 = 0b11010110
        )";

        auto root = toml::load(context);

        auto check = [&](const char* key, toml::toml_integer value) {
            REQUIRE(root.as_table().find(key)->second.as_integer() == value);
        };

        check("int1", 99);
        check("int2", 42);
        check("int3", 0);
        check("int4", -17);
        check("int5", 1000);
        check("int6", 5349221);
        check("int7", 5349221);
        check("int8", 12345);
        check("hex1", 0xDEADBEEF);
        check("hex2", 0xdeadbeef);
        check("hex3", 0xdeadbeef);
        check("oct1", 01234567);
        check("oct2", 0755);
        check("bin1", 0b11010110);
    }

    SECTION("invalid")
    {

    }
}

TEST_CASE("floating")
{
    SECTION("valid")
    {
        std::string context = R"(
            flt1 = +1.0
            flt2 = 3.1415
            flt3 = -0.01

            flt4 = 5e+22
            flt5 = 1e06
            flt6 = -2E-2

            flt7 = 6.626e-34
            sf1 = inf  
            sf2 = +inf 
            sf3 = -inf 

            sf4 = nan  
            sf5 = +nan 
            sf6 = -nan 
        )";

        auto root = toml::load(context);

        auto get_float = [&](const char* key) {
            return root.as_table().find(key)->second.as_float(); 
        };

        auto float_cmp = [](toml::toml_float f1, toml::toml_float f2) {
            REQUIRE(std::abs(f1 - f2) < 1e-5);
        };

        float_cmp(get_float("flt1"), 1.0);
        float_cmp(get_float("flt2"), 3.1415);
        float_cmp(get_float("flt3"), -0.01);
        float_cmp(get_float("flt4"), 5e22);
        float_cmp(get_float("flt5"), 1e6);
        float_cmp(get_float("flt6"), -2e-2);
        float_cmp(get_float("flt7"), 6.626e-34);

        auto check_inf = [&](const char* key, bool sign) {
            auto f = get_float(key);
            REQUIRE(std::isinf(f));
            REQUIRE(std::signbit(f) == sign);
        };

        check_inf("sf1", false);
        check_inf("sf2", false);
        check_inf("sf3", true);

        auto check_nan = [&](const char* key, bool sign) {
            auto f = get_float(key);
            REQUIRE(std::isnan(f));
            REQUIRE(std::signbit(f) == sign);
        };

        check_nan("sf4", false);
        check_nan("sf5", false);
        check_nan("sf6", true);

    }

    SECTION("invalid")
    {

    }
}


