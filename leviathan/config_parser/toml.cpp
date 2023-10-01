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

TEST_CASE("array")
{
    std::string s = R"(
        integers = [ 1, 2, 3 ]
        colors = [ "red", "yellow", "green" ]
        nested_array_of_ints = [ [ 1, 2 ], [3, 4, 5] ]
        nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
        # string_array = [ "all", 'strings', """are the same""", '''type''' ]

        # Mixed-type arrays are allowed
        numbers = [ 0.1, 0.2, 0.5, 1, 2, 5 ]
        contributors = [
        "Foo Bar <foo@example.com>",
        { name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux" } # OK
        ]
    )";

    auto root = toml::load(s);

    auto get_array = [&](const char* key) -> auto& {
        return root.as_table().find(key)->second.as_array();
    };

    auto& integers = get_array("integers");
    REQUIRE(integers[0].as_integer() == 1);
    REQUIRE(integers[1].as_integer() == 2);
    REQUIRE(integers[2].as_integer() == 3);

    auto& colors = get_array("colors");
    REQUIRE(colors[0].as_string() == "red");
    REQUIRE(colors[1].as_string() == "yellow");
    REQUIRE(colors[2].as_string() == "green");

    auto& nested_array_of_ints = get_array("nested_array_of_ints");
    auto check_array_equal = [](auto& arraylike, auto&& results) {
        REQUIRE(arraylike.size() == results.size());
        int size = results.size();
        for (int i = 0; i < size; ++i) 
        {
            REQUIRE(arraylike[i].as_integer() == results[i]);
        }
    };

    check_array_equal(nested_array_of_ints[0].as_array(), std::vector<int>{1, 2});
    check_array_equal(nested_array_of_ints[1].as_array(), std::vector<int>{3, 4, 5});

    auto& nested_mixed_array = get_array("nested_mixed_array");
    check_array_equal(nested_mixed_array[0].as_array(), std::vector<int>{1, 2});

    REQUIRE(nested_mixed_array[1].as_array()[0].as_string() == "a");
    REQUIRE(nested_mixed_array[1].as_array()[1].as_string() == "b");
    REQUIRE(nested_mixed_array[1].as_array()[2].as_string() == "c");

    // mixed-type
    auto& numbers = get_array("numbers");
    REQUIRE(numbers[0].as_float() == 0.1);
    REQUIRE(numbers[1].as_float() == 0.2);
    REQUIRE(numbers[2].as_float() == 0.5);
    REQUIRE(numbers[3].as_integer() == 1);
    REQUIRE(numbers[4].as_integer() == 2);
    REQUIRE(numbers[5].as_integer() == 5);

    auto& contributors = get_array("contributors");
    REQUIRE(contributors[0].as_string() == "Foo Bar <foo@example.com>");
    REQUIRE(contributors[1].as_table().find("name")->second.as_string() == "Baz Qux");
    REQUIRE(contributors[1].as_table().find("email")->second.as_string() == "bazqux@example.com");
    REQUIRE(contributors[1].as_table().find("url")->second.as_string() == "https://example.com/bazqux");
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

    std::string s = R"(
str1 = 'The quick brown fox jumps over the lazy dog.'

str2 = '''
The quick brown \


  fox jumps over \
    the lazy dog.'''

str3 = '''\ 
       The quick brown \
       fox jumps over \
       the lazy dog.\
       ''')";

    auto root = toml::load(s);
    auto& s1 = root.as_table().find("str1")->second.as_string();
    auto& s2 = root.as_table().find("str2")->second.as_string();
    auto& s3 = root.as_table().find("str3")->second.as_string();
    REQUIRE(s1 == s2);
    REQUIRE(s3 == s2);

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
    SECTION("valid")
    {
        std::string s = R"(
str1 = "The quick brown fox jumps over the lazy dog."

str2 = """
The quick brown \


  fox jumps over \
    the lazy dog."""

str3 = """\ 
       The quick brown \
       fox jumps over \
       the lazy dog.\
       """


str4 = """Here are two quotation marks: "". Simple enough."""
# str5 = """Here are three quotation marks: """."""  # INVALID
str5 = """Here are three quotation marks: ""\"."""
str6 = """Here are fifteen quotation marks: ""\"""\"""\"""\"""\"."""

# "This," she said, "is just a pointless statement."
str7 = """"This," she said, "is just a pointless statement.""""
        )";

        auto root = toml::load(s);
        auto& s1 = root.as_table().find("str1")->second.as_string();
        auto& s2 = root.as_table().find("str2")->second.as_string();
        auto& s3 = root.as_table().find("str3")->second.as_string();
        REQUIRE(s1 == s2);
        REQUIRE(s3 == s2);

        auto check_equal = [&](const char* key, const char* value) {
            auto& str = root.as_table().find(key)->second.as_string();
            REQUIRE(str == value);
        };

        check_equal("str4", R"(Here are two quotation marks: "". Simple enough.)");
        check_equal("str5", R"(Here are three quotation marks: ""\".)");
        check_equal("str6", R"(Here are fifteen quotation marks: ""\"""\"""\"""\"""\".)");
        check_equal("str7", R"("This," she said, "is just a pointless statement.")");

    }

    SECTION("invalid")
    {
        std::string strs[] = {
            R"(str5 = """Here are three quotation marks: """."""  # INVALID)"
        };

        REQUIRE_THROWS(toml::load(strs[0]));
    }
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

        std::string s4 = R"(
            points = [ { x = 1, y = 2, z = 3 },
                       { x = 7, y = 8, z = 9 },
                       { x = 2, y = 4, z = 8 } ]
        )";

        root = toml::load(s4);

        auto check_point = [&](int idx, int x, int y, int z) {
            auto& table = root.as_table().find("points")->second.as_array()[idx].as_table();
            REQUIRE(table.find("x")->second.as_integer() == x);
            REQUIRE(table.find("y")->second.as_integer() == y);
            REQUIRE(table.find("z")->second.as_integer() == z);
        };

        check_point(0, 1, 2, 3);
        check_point(1, 7, 8, 9);
        check_point(2, 2, 4, 8);

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

TEST_CASE("table array")
{
    SECTION("valid")
    {
        std::string s = R"(
            [[products]]
            name = "Hammer"
            sku = 738594937

            [[products]]  # empty table within the array

            [[products]]
            name = "Nail"
            sku = 284758393

            color = "gray"
        )";

        auto root = toml::load(s);

        auto& array = root.as_table().find("products")->second.as_array();

        REQUIRE(array.size() == 3);

        REQUIRE(array[0].as_table().find("name")->second.as_string() == "Hammer");
        REQUIRE(array[0].as_table().find("sku")->second.as_integer() == 738594937);
        
        REQUIRE(array[1].as_table().empty());

        REQUIRE(array[2].as_table().find("name")->second.as_string() == "Nail");
        REQUIRE(array[2].as_table().find("sku")->second.as_integer() == 284758393);
        REQUIRE(array[2].as_table().find("color")->second.as_string() == "gray");
    }

    SECTION("invalid")
    {
        std::string s1 = R"(
            [fruit.physical]  # subtable, but to which parent element should it belong?
            color = "red"
            shape = "round"

            [[fruit]]  # parser must throw an error upon discovering that "fruit" is an array rather than a table.
            name = "apple"
        )";

        REQUIRE_THROWS(toml::load(s1));

        std::string s2 = R"(
            # INVALID TOML DOC
            fruits = []

            [[fruits]] # Not allowed
        )";

        REQUIRE_THROWS(toml::load(s2));

       std::string s3 = R"(
            # INVALID TOML DOC
            [[fruits]]
            name = "apple"

            [[fruits.varieties]]
            name = "red delicious"

            # INVALID: This table conflicts with the previous array of tables
            [fruits.varieties]
            name = "granny smith"

            [fruits.physical]
            color = "red"
            shape = "round"

            # INVALID: This array of tables conflicts with the previous table
            [[fruits.physical]]
            color = "green"
        )";

        REQUIRE_THROWS(toml::load(s3));
    }
}

TEST_CASE("define table")
{
    SECTION("valid")
    {
        std::string s = R"(
            [x.y] # for this to work

            [x] # defining a super-table afterward is ok
        )";

        auto root = toml::load(s);

        REQUIRE(root.as_table().contains("x"));
    }

    SECTION("invalid")
    {
        std::string s = R"(
            [x]
            [x] # redefined table
        )";

        REQUIRE_THROWS(toml::load(s));
    }

}
