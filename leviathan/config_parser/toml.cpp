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

TEST_CASE("single line literal string")
{
    std::string strs[] = {
        R"(winpath  = 'C:\Users\nodejs\templates')",  
        R"(winpath2 = '\\ServerX\admin$\system32\')",  
        R"(quoted   = 'Tom "Dubs" Preston-Werner')",  
        R"(regex    = '<\i\c*\s*>')",  
    };


    auto check = [](std::string str, const char* key, const char* value) {
        auto root = toml::load(str);
        auto it = root.as_table().find(key);
        REQUIRE(it->second.as_string() == value);
    };

    check(strs[0], "winpath", R"(C:\Users\nodejs\templates)");
    check(strs[1], "winpath2", R"(\\ServerX\admin$\system32\)");
    check(strs[2], "quoted", R"(Tom "Dubs" Preston-Werner)");
    check(strs[3], "regex", R"(<\i\c*\s*>)");
}

TEST_CASE("multi line literal string")
{
    std::string strs[] = {
        R"(str  = '''I [dw]on't need \d{2} apples''')",  
        R"(str = '''Here are fifteen quotation marks: """""""""""""""''')",  
        R"(str   = ''''That,' she said, 'is still pointless.'''')",  
    };


    auto check = [](std::string str, const char* key, const char* value) {
        auto root = toml::load(str);
        auto it = root.as_table().find(key);
        REQUIRE(it->second.as_string() == value);
    };

    check(strs[0], "str", R"(I [dw]on't need \d{2} apples)");
    check(strs[1], "str", R"(Here are fifteen quotation marks: """"""""""""""")");
    check(strs[2], "str", R"('That,' she said, 'is still pointless.')");

    std::string multiline = R"(
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
''')";

    check(multiline, "lines", "The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n");
}

TEST_CASE("single line basic string")
{
    std::string strs[] = {
        R"(apos15 = "Here are fifteen apostrophes: '''''''''''''''")",
        "s = \"Roses are red\\nViolets are blue\"",
        "s = \"The quick brown fox jumps over the lazy dog.\"",
        R"(s = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")",
    };

    auto check = [](std::string str, const char* key, const char* value) {
        auto root = toml::load(str);
        auto it = root.as_table().find(key);
        REQUIRE(it->second.as_string() == value);
    };

    check(strs[0], "apos15", "Here are fifteen apostrophes: '''''''''''''''");
    check(strs[1], "s", "Roses are red\nViolets are blue");
    check(strs[2], "s", "The quick brown fox jumps over the lazy dog.");
    check(strs[3], "s", "I'm a string. \"You can quote me\". Name\tJosé\nLocation\tSF.");

}

TEST_CASE("multi line basic string")
{

}

TEST_CASE("inline table")
{
    SECTION("valid")
    {
        std::string s1 = "inline = { name = 'Alice', age = 10 }";

        auto root = toml::load(s1);

        REQUIRE(root.as_table().find("inline")->second.as_table().find("name")->second.as_string() == "Alice");
        REQUIRE(root.as_table().find("inline")->second.as_table().find("age")->second.as_integer() == 10);

        std::string s2 = "animal = { type.name = \"pug\" }";

        root = toml::load(s2);

        auto value = root.as_table().find("animal")->
                   second.as_table().find("type")->
                   second.as_table().find("name")->second.as_string();
        REQUIRE(value == "pug");

        std::string s3 = "empty_inline_table = {}";

        root = toml::load(s3);

        REQUIRE(root.as_table().find("empty_inline_table")->second.as_table().empty());
    }

    SECTION("invalid")
    {
        std::string s1 = "inline = { error = 'end with comma', }";
        std::string s2 = R"(
[product]
type = { name = "Nail" }
type.edible = false  # INVALID
        )";

        REQUIRE_THROWS(toml::load(s1));
        REQUIRE_THROWS(toml::load(s2));
    }
}
