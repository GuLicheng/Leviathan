// https://github.com/toml-lang/toml-test/tree/master
// https://www.bejson.com/validators/toml_editor/

#include "toml.hpp"
#include "../json/json.hpp"
#include "../value_cast.hpp"

#include <catch2/catch_all.hpp>

#include <leviathan/print.hpp>
#include <leviathan/extc++/all.hpp>
#include <numeric>
#include <iostream>
#include <filesystem>
#include <functional>
#include <ranges>
#include <generator>
#include <span>

namespace toml = cpp::toml;
namespace json = cpp::json;

using JsonDecoder = json::decoder;
using TomlDecoder = toml::decoder;
using JsonFormatter = cpp::config::toml2json;

toml::value ParseAsTomlValue(const char* context)
{
    return TomlDecoder(context).parse_val();
}

template <typename TomlValueType, typename Expected>
bool TestSingleValue(const char* context, Expected expected)
{
    auto tv = ParseAsTomlValue(context);
    return tv.is<TomlValueType>() && tv.as<TomlValueType>() == expected;
}

template <typename TomlValueType, typename Expected, typename... Keys>
bool TestSingleKeyValue(const char* context, const char* key, Expected expected)
{
    auto tv = ParseAsTomlValue(context);
    auto it = tv.as<toml::table>().find(key);
    return it->second.is<TomlValueType>() && it->second.as<TomlValueType>() == expected;
}

TEST_CASE("valid-boolean")
{
    SECTION("valid")
    {
        const char* contexts[] = 
        {
            "true",
            "false"
        };

        CHECK(TestSingleValue<toml::boolean>(contexts[0], true));
        CHECK(TestSingleValue<toml::boolean>(contexts[1], false));
    }
}

TEST_CASE("valid-floating")
{
    auto TomlFloatingCompare = [](const char* context, double actual) static
    {
        return TestSingleValue<toml::floating>(context, actual);
    };

    // Zero
    CHECK(TomlFloatingCompare("0.0", 0));
    CHECK(TomlFloatingCompare("+0.0", 0));
    CHECK(TomlFloatingCompare("-0.0", 0));
    CHECK(TomlFloatingCompare("0e0", 0));
    CHECK(TomlFloatingCompare("0e00", 0));
    CHECK(TomlFloatingCompare("+0e0", 0));
    CHECK(TomlFloatingCompare("-0e0", 0));

    // Underscore
    CHECK(TomlFloatingCompare("3_141.5927", 3'141.5927));
    CHECK(TomlFloatingCompare("3141.592_7", 3141.592'7));
    CHECK(TomlFloatingCompare("3e1_4", 3e1'4));

    // Max-int
    CHECK(TomlFloatingCompare("9_007_199_254_740_991.0", 9'007'199'254'740'991.0));
    CHECK(TomlFloatingCompare("-9_007_199_254_740_991.0", -9'007'199'254'740'991.0));

    // Long
    CHECK(TomlFloatingCompare("3.141592653589793", 3.141592653589793));
    CHECK(TomlFloatingCompare("-3.141592653589793", -3.141592653589793));

    // Inf-and-nan

    auto IsTomlFloatingInf = [](const char* context) static
    {
        auto tv = ParseAsTomlValue(context);
        // Does sign matter?
        return tv.is<toml::floating>() 
            && std::isinf(tv.as<toml::floating>());
    };

    CHECK(IsTomlFloatingInf("inf"));
    CHECK(IsTomlFloatingInf("+inf"));
    CHECK(IsTomlFloatingInf("-inf"));

    auto IsTomlFloatingNaN = [](const char* context) static
    {
        auto tv = ParseAsTomlValue(context);
        // Does sign matter?
        return tv.is<toml::floating>() 
            && std::isnan(tv.as<toml::floating>());
    };

    CHECK(IsTomlFloatingNaN("nan"));
    CHECK(IsTomlFloatingNaN("+nan"));
    CHECK(IsTomlFloatingNaN("-nan"));

    // Float
    CHECK(TomlFloatingCompare("3.14", 3.14));
    CHECK(TomlFloatingCompare("+3.14", 3.14));
    CHECK(TomlFloatingCompare("-3.14", -3.14));
    CHECK(TomlFloatingCompare("0.123", 0.123));

    // Exponent
    CHECK(TomlFloatingCompare("3e2", 3e2));
    CHECK(TomlFloatingCompare("3E2", 3e2));
    CHECK(TomlFloatingCompare("3e-2", 3e-2));
    CHECK(TomlFloatingCompare("3E+2", 3e+2));
    CHECK(TomlFloatingCompare("3e0", 3e0));
    CHECK(TomlFloatingCompare("3.1e2", 3.1e2));
    CHECK(TomlFloatingCompare("3.1E2", 3.1E2));
    CHECK(TomlFloatingCompare("-1E-1", -1E-1));
}

TEST_CASE("valid-integer")
{
    auto TomlIntegerCompare = [](const char* context, int64_t actual) static
    {
        return TestSingleValue<toml::integer>(context, actual);
    };

    // Zero
    CHECK(TomlIntegerCompare("0", 0));
    CHECK(TomlIntegerCompare("+0", 0));
    CHECK(TomlIntegerCompare("-0", 0));
    CHECK(TomlIntegerCompare("0x0", 0));
    CHECK(TomlIntegerCompare("0x00", 0));
    CHECK(TomlIntegerCompare("0x00000", 0));
    CHECK(TomlIntegerCompare("0o0", 0));
    CHECK(TomlIntegerCompare("0o00", 0));
    CHECK(TomlIntegerCompare("0o00000", 0));
    CHECK(TomlIntegerCompare("0b0", 0));
    CHECK(TomlIntegerCompare("0b00", 0));
    CHECK(TomlIntegerCompare("0b00000", 0));

    // Underscore
    CHECK(TomlIntegerCompare("1_000", 1'000));
    CHECK(TomlIntegerCompare("1_1_1_1", 1'1'1'1));

    // Long
    CHECK(TomlIntegerCompare("9223372036854775807", std::numeric_limits<int64_t>::max()));
    CHECK(TomlIntegerCompare("-9223372036854775808", std::numeric_limits<int64_t>::min()));

    // Integer
    CHECK(TomlIntegerCompare("42", 42));
    CHECK(TomlIntegerCompare("+42", 42));
    CHECK(TomlIntegerCompare("-42", -42));
    CHECK(TomlIntegerCompare("0", 0));

    // Literals
    CHECK(TomlIntegerCompare("0b11010110", 0b11010110));
    CHECK(TomlIntegerCompare("0b1_0_1", 0b1'0'1));
    CHECK(TomlIntegerCompare("0o01234567", 01234567));
    CHECK(TomlIntegerCompare("0o755", 0755));
    CHECK(TomlIntegerCompare("0o7_6_5", 0765));
    CHECK(TomlIntegerCompare("0xDEADBEEF", 0xDEADBEEF));
    CHECK(TomlIntegerCompare("0xdeadbeef", 0xdeadbeef));
    CHECK(TomlIntegerCompare("0xdead_beef", 0xdeadbeef));
    CHECK(TomlIntegerCompare("0x00987", 0x00987));

    // Float64-max
    CHECK(TomlIntegerCompare("9_007_199_254_740_991", 9'007'199'254'740'991));
    CHECK(TomlIntegerCompare("-9_007_199_254_740_991", -9'007'199'254'740'991));
}

bool CheckTomlDatetime(const char* context,
        int year, int month, int day, 
        int hour = 0, int minute = 0, int second = 0, 
        int nanosecond = 0, int offset = 0)
{
    auto root = ParseAsTomlValue(context);
    auto& dt = root.as<toml::datetime>();
    auto [date, time, off] = dt;

    return (date.m_year == year)
        && (date.m_month == month)
        && (date.m_day == day)
        && (time.m_hour == hour)
        && (time.m_minute == minute)
        && (time.m_second == second)
        && (off.m_minute == offset);
}

TEST_CASE("valid-datetime")
{
    CHECK(CheckTomlDatetime("1987-07-05 17:45:00Z", 1987, 7, 5, 17, 45, 0));
    CHECK(CheckTomlDatetime("1987-07-05t17:45:00z", 1987, 7, 5, 17, 45, 0));

    CHECK(CheckTomlDatetime("0001-01-01 00:00:00Z", 1, 1, 1));
    CHECK(CheckTomlDatetime("0001-01-01 00:00:00", 1, 1, 1));
    CHECK(CheckTomlDatetime("0001-01-01", 1, 1, 1));

    CHECK(CheckTomlDatetime("9999-12-31 23:59:59Z", 9999, 12, 31, 23, 59, 59));
    CHECK(CheckTomlDatetime("9999-12-31 23:59:59", 9999, 12, 31, 23, 59, 59));
    CHECK(CheckTomlDatetime("9999-12-31", 9999, 12, 31));

    CHECK(CheckTomlDatetime("2000-02-29 15:15:15Z", 2000, 2, 29, 15, 15, 15));
    CHECK(CheckTomlDatetime("2000-02-29 15:15:15", 2000, 2, 29, 15, 15, 15));
    CHECK(CheckTomlDatetime("2000-02-29", 2000, 2, 29));

    CHECK(CheckTomlDatetime("2024-02-29 15:15:15Z", 2024, 2, 29, 15, 15, 15));
    CHECK(CheckTomlDatetime("2024-02-29 15:15:15", 2024, 2, 29, 15, 15, 15));
    CHECK(CheckTomlDatetime("2024-02-29", 2024, 2, 29));

    CHECK(CheckTomlDatetime("1987-07-05", 1987, 7, 5));
    CHECK(CheckTomlDatetime("17:45:00", 0, 0, 0, 17, 45));
    CHECK(CheckTomlDatetime("10:32:00.555", 0, 0, 0, 10, 32, 0, 555));

    CHECK(CheckTomlDatetime("1987-07-05T17:45:00", 1987, 7, 5, 17, 45));
    CHECK(CheckTomlDatetime("1977-12-21T10:32:00.555", 1977, 12, 21, 10, 32, 0, 555));
    CHECK(CheckTomlDatetime("1987-07-05 17:45:00", 1987, 7, 5, 17, 45));

    CHECK(CheckTomlDatetime("1987-07-05T17:45:56.123Z", 1987, 7, 5, 17, 45, 56, 123));
    CHECK(CheckTomlDatetime("1987-07-05T17:45:56.6Z", 1987, 7, 5, 17, 45, 56, 6));
    CHECK(CheckTomlDatetime("1987-07-05T17:45:56.123+08:00", 1987, 7, 5, 17, 45, 56, 123, 480));
    CHECK(CheckTomlDatetime("1987-07-05T17:45:56.6+08:00", 1987, 7, 5, 17, 45, 56, 6, 480));
    
    CHECK(CheckTomlDatetime("13:37", 0, 0, 0, 13, 37));
    CHECK(CheckTomlDatetime("1979-05-27 07:32Z", 1979, 5, 27, 7, 32));
    CHECK(CheckTomlDatetime("1979-05-27 07:32-07:00", 1979, 5, 27, 7, 32, 0, 0, -420));
    CHECK(CheckTomlDatetime("1979-05-27T07:32", 1979, 5, 27, 7, 32));

    CHECK(CheckTomlDatetime("1987-07-05T17:45:56Z", 1987, 7, 5, 17, 45, 56));
    CHECK(CheckTomlDatetime("1987-07-05T17:45:56-05:00", 1987, 7, 5, 17, 45, 56, 0, -300));
    CHECK(CheckTomlDatetime("1987-07-05T17:45:56+12:00", 1987, 7, 5, 17, 45, 56, 0, 720));
    CHECK(CheckTomlDatetime("1987-07-05T17:45:56+13:00", 1987, 7, 5, 17, 45, 56, 0, 780));

}

const toml::value* ExtractOneElementByIndex(const toml::value& root, const char* key)
{
    return &(root.as<toml::table>().find(key)->second);
}

const toml::value* ExtractOneElementByIndex(const toml::value& root, size_t index)
{
    return &(root.as<toml::array>().at(index));
}

template <typename Key1, typename... Keys>
const toml::value* SearchTomlTableOrArray(const toml::value& root, Key1 key1, Keys... keys)
{
    if constexpr (sizeof...(Keys) == 0)
    {
        return ExtractOneElementByIndex(root, key1);
    }
    else
    {
        const toml::value* next = ExtractOneElementByIndex(root, key1);
        return SearchTomlTableOrArray(*next, keys...);
    }
}

template <typename TomlValueType, typename Actual, typename... Keys>
bool LoadTomlValueAndExtract(const char* context, Actual actual, Keys... keys)
{
    auto tv = ParseAsTomlValue(context);
    const toml::value* node = SearchTomlTableOrArray(tv, keys...);
    return node->is<TomlValueType>() && node->as<TomlValueType>() == actual;
}

template <typename TomlValueType, typename Actual, typename... Keys>
bool LoadTomlValueAndExtract(const toml::value& root, Actual actual, Keys... keys)
{
    const toml::value* node = SearchTomlTableOrArray(root, keys...);
    return node->is<TomlValueType>() && node->as<TomlValueType>() == actual;
}

TEST_CASE("valid-array")
{
    const char* context = "[true, false]";
    CHECK(LoadTomlValueAndExtract<toml::boolean>(context, true, 0));
    CHECK(LoadTomlValueAndExtract<toml::boolean>(context, false, 1));

    context = "[[1, 2], [\"a\", \"b\"], [1.1, 2.1]]";
    CHECK(LoadTomlValueAndExtract<toml::integer>(context, 1, 0, 0));
    CHECK(LoadTomlValueAndExtract<toml::integer>(context, 2, 0, 1));
    CHECK(LoadTomlValueAndExtract<toml::string>(context, "a", 1, 0));
    CHECK(LoadTomlValueAndExtract<toml::string>(context, "b", 1, 1));
    CHECK(LoadTomlValueAndExtract<toml::floating>(context, 1.1, 2, 0));
    CHECK(LoadTomlValueAndExtract<toml::floating>(context, 2.1, 2, 1));

    context = "[1, [\"Arrays are not integers.\"]]";
    CHECK(LoadTomlValueAndExtract<toml::integer>(context, 1, 0));
    CHECK(LoadTomlValueAndExtract<toml::string>(context, "Arrays are not integers.", 1, 0));

    context = "[1, 1.1]";
    CHECK(LoadTomlValueAndExtract<toml::integer>(context, 1, 0));
    CHECK(LoadTomlValueAndExtract<toml::floating>(context, 1.1, 1));

    context = "[\"hi\", 42]";
    CHECK(LoadTomlValueAndExtract<toml::string>(context, "hi", 0));
    CHECK(LoadTomlValueAndExtract<toml::integer>(context, 42, 1));

    context = R"([["a"], ["b"]])";
    CHECK(LoadTomlValueAndExtract<toml::string>(context, "a", 0, 0));
    CHECK(LoadTomlValueAndExtract<toml::string>(context, "b", 1, 0));

    context = R"([ { bar="\"{{baz}}\""} ])";
    CHECK(LoadTomlValueAndExtract<toml::string>(context, "\"{{baz}}\"", 0, "bar"));

    context = R"([ { b = {} } ])";
    auto root = ParseAsTomlValue(context);
    auto t = SearchTomlTableOrArray(root, 0, "b");
    REQUIRE(t->is<toml::table>());
    REQUIRE(t->as<toml::table>().empty());

    context = R"(
        nested-array = [[[[[]]]]]
        contributors = [
            "Foo Bar <foo@example.com>",
            { name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux" }
        ]

        # Start with a table as the first element. This tests a case that some libraries
        # might have where they will check if the first entry is a table/map/hash/assoc
        # array and then encode it as a table array. This was a reasonable thing to do
        # before TOML 1.0 since arrays could only contain one type, but now it's no
        # longer.
        mixed = [{k="a"}, "b", 1]
    )";

    root = toml::loads(context);
    auto nested_array = SearchTomlTableOrArray(root, "nested-array", 0, 0, 0, 0);
    REQUIRE(nested_array->is<toml::array>());
    REQUIRE(nested_array->as<toml::array>().empty());

    REQUIRE(LoadTomlValueAndExtract<toml::string>(root, "a", "mixed", 0, "k"));
    REQUIRE(LoadTomlValueAndExtract<toml::string>(root, "b", "mixed", 1));
    REQUIRE(LoadTomlValueAndExtract<toml::integer>(root, 1, "mixed", 2));

    CHECK(LoadTomlValueAndExtract<toml::string>(root, "Foo Bar <foo@example.com>", "contributors", 0));
    CHECK(LoadTomlValueAndExtract<toml::string>(root, "Baz Qux", "contributors", 1, "name"));
    CHECK(LoadTomlValueAndExtract<toml::string>(root, "bazqux@example.com", "contributors", 1, "email"));
    CHECK(LoadTomlValueAndExtract<toml::string>(root, "https://example.com/bazqux", "contributors", 1, "url"));
}

TEST_CASE("valid-inline-table")
{
    SECTION("simple")
    {
        constexpr const char* context = R"(
    
a = { a = [
]}

b = { a = [
		1,
		2,
	], b = [
		3,
		4,
	]}

arr = [ {'a'= 1}, {'a'= 2} ]

people = [{first_name = "Bruce", last_name = "Springsteen"},
          {first_name = "Eric", last_name = "Clapton"},
          {first_name = "Bob", last_name = "Seger"}]
    
c = {a = true, b = false}

empty1 = {}
empty2 = { }
empty_in_array = [ { not_empty = 1 }, {} ]
empty_in_array2 = [{},{not_empty=1}]
many_empty = [{},{},{}]
nested_empty = {"empty"={}}
with_cmt ={            }#nothing here

black = { python=">3.6", version=">=18.9b0", allow_prereleases=true }


name        = { first = "Tom", last = "Preston-Werner" }
point       = { x = 1, y = 2 }
simple      = { a = 1 }
str-key     = { "a" = 1 }
table-array = [{ "a" = 1 }, { "b" = 2 }]


tbl_multiline = { a = 1, b = """
multiline
""", c = """and yet
another line""", d = 4 }

# https://github.com/toml-lang/toml-test/issues/146
clap-1 = { version = "4"  , features = ["derive", "cargo"] }

# Contains some literal tabs!
clap-2 = { version = "4"	   	,	  	features = [   "derive" 	  ,  	  "cargo"   ]   , nest   =   {  	  "a"   =   'x'  , 	  'b'   = [ 1.5    ,   9.0  ]  }  }

tbl_tbl_empty = { tbl_0 = {} }
tbl_tbl_val   = { tbl_1 = { one = 1 } }
tbl_arr_tbl   = { arr_tbl = [ { one = 1 } ] }
arr_tbl_tbl   = [ { tbl = { one = 1 } } ]

# Array-of-array-of-table is interesting because it can only
# be represented in inline form.
arr_arr_tbl_empty = [ [ {} ] ]
arr_arr_tbl_val = [ [ { one = 1 } ] ]
arr_arr_tbls  = [ [ { one = 1 }, { two = 2 } ] ]

    )";

        auto root = toml::loads(context);

        CHECK(SearchTomlTableOrArray(root, "tbl_tbl_empty", "tbl_0")->as<toml::table>().empty());
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "tbl_tbl_val", "tbl_1", "one"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "tbl_arr_tbl", "arr_tbl", 0, "one"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr_tbl_tbl", 0, "tbl", "one"));

        CHECK(SearchTomlTableOrArray(root, "arr_arr_tbl_empty", 0, 0)->as<toml::table>().empty());
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr_arr_tbl_val", 0, 0, "one"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr_arr_tbls", 0, 0, "one"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "arr_arr_tbls", 0, 1, "two"));

        CHECK(LoadTomlValueAndExtract<toml::string>(root, "4", "clap-2", "version"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "derive", "clap-2", "features", 0));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "cargo", "clap-2", "features", 1));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "x", "clap-2", "nest", "a"));
        CHECK(LoadTomlValueAndExtract<toml::floating>(root, 1.5, "clap-2", "nest", "b", 0));
        CHECK(LoadTomlValueAndExtract<toml::floating>(root, 9.0, "clap-2", "nest", "b", 1));

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "tbl_multiline", "a"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "multiline\n", "tbl_multiline", "b"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "and yet\nanother line", "tbl_multiline", "c"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 4, "tbl_multiline", "d"));

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "table-array", 0, "a"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "table-array", 1, "b"));

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "str-key", "a"));

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "simple", "a"));

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "point", "x"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "point", "y"));

        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Tom", "name", "first"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Preston-Werner", "name", "last"));

        CHECK(LoadTomlValueAndExtract<toml::string>(root, ">3.6", "black", "python"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, ">=18.9b0", "black", "version"));
        CHECK(LoadTomlValueAndExtract<toml::boolean>(root, true, "black", "allow_prereleases"));

        CHECK(SearchTomlTableOrArray(root, "with_cmt")->as<toml::table>().empty());
        CHECK(SearchTomlTableOrArray(root, "nested_empty", "empty")->as<toml::table>().empty());

        CHECK(SearchTomlTableOrArray(root, "many_empty", 0)->as<toml::table>().empty());
        CHECK(SearchTomlTableOrArray(root, "many_empty", 1)->as<toml::table>().empty());
        CHECK(SearchTomlTableOrArray(root, "many_empty", 2)->as<toml::table>().empty());

        CHECK(SearchTomlTableOrArray(root, "a", "a")->as<toml::array>().empty());
        
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "b", "a", 0));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "b", "a", 1));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 3, "b", "b", 0));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 4, "b", "b", 1));

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr", 0, "a"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "arr", 1, "a"));

        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Bruce", "people", 0, "first_name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Springsteen", "people", 0, "last_name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Eric", "people", 1, "first_name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Clapton", "people", 1, "last_name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Bob", "people", 2, "first_name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Seger", "people", 2, "last_name"));

        CHECK(LoadTomlValueAndExtract<toml::boolean>(root, true, "c", "a"));
        CHECK(LoadTomlValueAndExtract<toml::boolean>(root, false, "c", "b"));

        CHECK(SearchTomlTableOrArray(root, "empty1")->as<toml::table>().empty());
        CHECK(SearchTomlTableOrArray(root, "empty2")->as<toml::table>().empty());

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "empty_in_array", 0, "not_empty"));
        CHECK(SearchTomlTableOrArray(root, "empty_in_array", 1)->as<toml::table>().empty());
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "empty_in_array2", 1, "not_empty"));
        CHECK(SearchTomlTableOrArray(root, "empty_in_array2", 0)->as<toml::table>().empty());
    }

    SECTION("key-dotted-1")
    {
        constexpr const char* context = R"(
            a = {   a.b  =  1   }
            b = {   "a"."b"  =  1   }
            c = {   a   .   b  =  1   }
            d = {   'a'   .   "b"  =  1   }
            e = {a.b=1}
        )";

        auto root = toml::loads(context);

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "a", "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "b", "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "c", "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "d", "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "e", "a", "b"));
    }

    SECTION("key-dotted-2")
    {
        constexpr const char* context = "many.dots.here.dot.dot.dot = {a.b.c = 1, a.b.d = 2}";
        auto root = toml::loads(context);
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "many", "dots", "here", "dot", "dot", "dot", "a", "b", "c"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "many", "dots", "here", "dot", "dot", "dot", "a", "b", "d"));
    }

    SECTION("key-dotted-3")
    {
        constexpr const char* context = R"(
            [tbl]
            a.b.c = {d.e=1}

            [tbl.x]
            a.b.c = {d.e=1}
        )";

        auto root = toml::loads(context);
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "tbl", "a", "b", "c", "d", "e"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "tbl", "x", "a", "b", "c", "d", "e"));
    }

    SECTION("key-dotted-4")
    {
        constexpr const char* context = R"(
            [[arr]]
            t = {a.b=1}
            T = {a.b=1}

            [[arr]]
            t = {a.b=2}
            T = {a.b=2}
        )";
        auto root = toml::loads(context);
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr", 0, "t", "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr", 0, "T", "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "arr", 1, "t", "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "arr", 1, "T", "a", "b"));
    }

    SECTION("key-dotted-5")
    {
        constexpr const char* context = R"(
            arr-1 = [{a.b = 1}]
            arr-2 = ["str", {a.b = 1}]

            arr-3 = [{a.b = 1}, {a.b = 2}]
            arr-4 = ["str", {a.b = 1}, {a.b = 2}]
            top.dot.dot = [
                {dot.dot.dot = 1},
                {dot.dot.dot = 2},
            ]
            arr = [
                {a.b = [{c.d = 1}]}
            ]
        )";
        auto root = toml::loads(context);
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr-1", 0, "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "str", "arr-2", 0));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr-2", 1, "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr-3", 0, "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "arr-3", 1, "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "str", "arr-4", 0));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr-4", 1, "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "arr-4", 2, "a", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "top", "dot", "dot", 0, "dot", "dot", "dot"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "top", "dot", "dot", 1, "dot", "dot", "dot"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr", 0, "a", "b", 0, "c", "d"));
    }
}

TEST_CASE("valid-comment")
{
    SECTION("after-no-literal-no-ws")
    {
        const char* contexts = R"(
            true=true#true
            false=false#false
        )";
        auto root = toml::loads(contexts);
        CHECK(LoadTomlValueAndExtract<toml::boolean>(root, true, "true"));
        CHECK(LoadTomlValueAndExtract<toml::boolean>(root, false, "false"));

        contexts = R"( 
#This is a full-line comment
key = "value" # This is a comment at the end of a line)";

        root = toml::loads(contexts);
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "value", "key"));

        contexts = "# single comment without any eol characters";
        root = toml::loads(contexts);
    }

    SECTION("everywhere")
    {
        constexpr const char* contexts = R"(
# Top comment.
  # Top comment.
# Top comment.

# [no-extraneous-groups-please]

[group] # Comment
answer = 42 # Comment
# no-extraneous-keys-please = 999
# Inbetween comment.
more = [ # Comment
  # What about multiple # comments?
  # Can you handle it?
  #
          # Evil.
# Evil.
  42, 42, # Comments within arrays are fun.
  # What about multiple # comments?
  # Can you handle it?
  #
          # Evil.
# Evil.
# ] Did I fool you?
] # Hopefully not.

# Make sure the space between the datetime and "#" isn't lexed.
dt = 1979-05-27T07:32:12-07:00  # c
d = 1979-05-27 # Comment
        )";
        auto root = toml::loads(contexts);
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 42, "group", "answer"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 42, "group", "more", 0));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 42, "group", "more", 1));
        
        auto& dt = SearchTomlTableOrArray(root, "group", "dt")->as<toml::datetime>();
        CHECK(dt.m_data.m_year == 1979);
        CHECK(dt.m_data.m_month == 5);
        CHECK(dt.m_data.m_day == 27);
        CHECK(dt.m_time.m_hour == 7);
        CHECK(dt.m_time.m_minute == 32);
        CHECK(dt.m_time.m_second == 12);
        CHECK(dt.m_offset.m_minute == -420);

        auto& d = SearchTomlTableOrArray(root, "group", "d")->as<toml::datetime>();
        CHECK(d.m_data.m_year == 1979);
        CHECK(d.m_data.m_month == 5);
        CHECK(d.m_data.m_day == 27);
    }
}

TEST_CASE("valid-key")
{
    SECTION("1")
    {

        constexpr const char* contexts = R"(
            alpha = "a"
            123 = "num"
            000111 = "leading"
            10e3 = "false float"
            one1two2 = "mixed"
            with-dash = "dashed"
            under_score = "___"
            34-11 = 23

            [2018_10]
            001 = 1

            [a-a-a]
            _ = false

        )";

        auto root = toml::loads(contexts);
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "a", "alpha"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "num", "123"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "leading", "000111"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "false float", "10e3"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "mixed", "one1two2"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "dashed", "with-dash"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "___", "under_score"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 23, "34-11"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "2018_10", "001"));
        CHECK(LoadTomlValueAndExtract<toml::boolean>(root, false, "a-a-a", "_"));
    }

    SECTION("2")
    {
        constexpr const char* contexts = R"(
            sectioN = "NN"

            [section]
            name = "lower"
            NAME = "upper"
            Name = "capitalized"

            [Section]
            name = "different section!!"
            "μ" = "greek small letter mu"
            "Μ" = "greek capital letter MU"
            M = "latin letter M"
        )";
        auto root = toml::loads(contexts);

        CHECK(LoadTomlValueAndExtract<toml::string>(root, "NN", "sectioN"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "lower", "section", "name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "upper", "section", "NAME"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "capitalized", "section", "Name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "different section!!", "Section", "name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "greek small letter mu", "Section", "μ"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "greek capital letter MU", "Section", "Μ"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "latin letter M", "Section", "M"));
    }

    SECTION("3")
    {
        constexpr const char* contexts = R"(
            name.first = "Arthur"
            "name".'last' = "Dent"

            # Note: this file contains literal tab characters.

            # Space are ignored, and key parts can be quoted.
            count.a       = 1
            count . b     = 2
            "count"."c"   = 3
            "count" . "d" = 4
            'count'.'e'   = 5
            'count' . 'f' = 6
            "count".'g'   = 7
            "count" . 'h' = 8
            count.'i'     = 9
            count 	.	 'j'	   = 10
            "count".k     = 11
            "count" . l   = 12

top.key = 1

''.x = "empty.x"
x."" = "x.empty"

[tbl]
a.b.c = 42.666

[a.few.dots]
polka.dot = "again?"
polka.dance-with = "Dot"

[[arr]]
a.b.c=1
a.b.d=2

[[arr]]
a.b.c=3
a.b.d=4


[a]
"".'' = "empty.empty"

        )";
        auto root = toml::loads(contexts);

        CHECK(LoadTomlValueAndExtract<toml::string>(root, "empty.x", "", "x"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "x.empty", "x", ""));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "empty.empty", "a", "", ""));

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "arr", 0, "a", "b", "c"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "arr", 0, "a", "b", "d"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 3, "arr", 1, "a", "b", "c"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 4, "arr", 1, "a", "b", "d"));

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "top", "key"));
        CHECK(LoadTomlValueAndExtract<toml::floating>(root, 42.666, "tbl", "a", "b", "c"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "again?", "a", "few", "dots", "polka", "dot"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Dot", "a", "few", "dots", "polka", "dance-with"));
        
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Arthur", "name", "first"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Dent", "name", "last"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "count", "a"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "count", "b"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 3, "count", "c"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 4, "count", "d"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 5, "count", "e"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 6, "count", "f"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 7, "count", "g"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 8, "count", "h"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 9, "count", "i"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 10, "count", "j"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 11, "count", "k"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 12, "count", "l"));

    }

    SECTION("empty")
    {
        constexpr const char* contexts[] = 
        {
            R"("" = "blank")",
            R"('' = "blank")",
            R"(''=0)",
            R"(1.2 = 3)",
            R"(1 = 1)",
        };
    
        CHECK(SearchTomlTableOrArray(toml::loads(contexts[0]), "")->as<toml::string>() == "blank");
        CHECK(SearchTomlTableOrArray(toml::loads(contexts[1]), "")->as<toml::string>() == "blank");
        CHECK(SearchTomlTableOrArray(toml::loads(contexts[2]), "")->as<toml::integer>() == 0);
        CHECK(SearchTomlTableOrArray(toml::loads(contexts[3]), "1", "2")->as<toml::integer>() == 3);
        CHECK(SearchTomlTableOrArray(toml::loads(contexts[4]), "1")->as<toml::integer>() == 1);
    }

    SECTION("4")
    {
        constexpr const char* context = R"(
plain = 1
"with.dot" = 2

"=~!@$^&*()_+-`1234567890[]|/?><.,;:'=" = 1

[plain_table]
plain = 3
"with.dot" = 4

[table.withdot]
plain = 5
"key.with.dots" = 6

[-]
- = 4

[_]
_ = 5

        )";
        auto root = toml::loads(context);
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "=~!@$^&*()_+-`1234567890[]|/?><.,;:'="));
        
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "plain"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "with.dot"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 3, "plain_table", "plain"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 4, "plain_table", "with.dot"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 5, "table", "withdot", "plain"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 6, "table", "withdot", "key.with.dots"));

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 4, "-", "-"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 5, "_", "_"));
    }
}

TEST_CASE("valid-table")
{
    SECTION("array-1")
    {
        constexpr const char* contexts = R"(
[[a.b]]
x = 1

[a]
y = 2
[[albums.songs]]
name = "Glory Days"
[[people]]
first_name = "Bruce"
last_name = "Springsteen"

[[people]]
first_name = "Eric"
last_name = "Clapton"

[[people]]
first_name = "Bob"
last_name = "Seger"
        )";

        auto root = toml::loads(contexts);
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "a", "b", 0, "x"));        
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "a", "y"));        
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Glory Days", "albums", "songs", 0, "name"));  

        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Bruce", "people", 0, "first_name"));        
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Springsteen", "people", 0, "last_name"));        
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Eric", "people", 1, "first_name"));        
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Clapton", "people", 1, "last_name"));        
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Bob", "people", 2, "first_name"));        
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Seger", "people", 2, "last_name"));        
    }

    SECTION("array-2")
    {
        constexpr const char* contexts = R"(
[[albums]]
name = "Born to Run"

  [[albums.songs]]
  name = "Jungleland"

  [[albums.songs]]
  name = "Meeting Across the River"

[[albums]]
name = "Born in the USA"
  
  [[albums.songs]]
  name = "Glory Days"

  [[albums.songs]]
  name = "Dancing in the Dark"

[[a]]
    [[a.b]]
        [a.b.c]
            d = "val0"
    [[a.b]]
        [a.b.c]
            d = "val1"
[fruit]
apple.color = "red"

[[fruit.apple.seeds]]
size = 2
        )";

        auto root = toml::loads(contexts);

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "fruit", "apple", "seeds", 0, "size"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "red", "fruit", "apple", "color"));

        CHECK(LoadTomlValueAndExtract<toml::string>(root, "val1", "a", 0, "b", 1, "c", "d"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "val0", "a", 0, "b", 0, "c", "d"));

        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Born to Run", "albums", 0, "name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Jungleland", "albums", 0, "songs", 0, "name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Meeting Across the River", "albums", 0, "songs", 1, "name"));

        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Born in the USA", "albums", 1, "name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Glory Days", "albums", 1, "songs", 0, "name"));
        CHECK(LoadTomlValueAndExtract<toml::string>(root, "Dancing in the Dark", "albums", 1, "songs", 1, "name"));

    }

    SECTION("empty-1")
    {
        constexpr const char* contexts = R"(
            ['']
            x = 1

            ["".a]
            x = 2

            [a.'']
            x = 3
        )";

        auto root = toml::loads(contexts);

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "", "x"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "", "a", "x"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 3, "a", "", "x"));
    }

    SECTION("empty-2")
    {
        constexpr const char* contexts = R"(
            [a]
        )";
        auto root = toml::loads(contexts);
        REQUIRE(SearchTomlTableOrArray(root, "a")->as<toml::table>().empty());
    }

    SECTION("keyword")
    {
        constexpr const char* contexts = R"(
            [true]

            [false]

            [inf]

            [nan]
        )";
        auto root = toml::loads(contexts);
        REQUIRE(SearchTomlTableOrArray(root, "true")->as<toml::table>().empty());
        REQUIRE(SearchTomlTableOrArray(root, "false")->as<toml::table>().empty());
        REQUIRE(SearchTomlTableOrArray(root, "inf")->as<toml::table>().empty());
        REQUIRE(SearchTomlTableOrArray(root, "nan")->as<toml::table>().empty());
    }

    SECTION("table-only")
    {
        auto CheckTomlEmptyTable = []<typename... Keys>(const toml::value& root, Keys... keys) static
        {
            return SearchTomlTableOrArray(root, keys...)->template as<toml::table>().empty();
        };

        constexpr const char* context = R"(
[a.b.c]
[a."b.c"]
[a.'d.e']
[a.' x ']
[ d.e.f ]
[ g . h . i ]
[ j . "ʞ" . 'l' ]

[x.1.2]
[x] # defining a super-table afterwards is ok

[aa]
key = 1

# a.extend is a key inside the "a" table.
[aa.extend]
key = 2

[aa.extend.more]
key = 3

['a1']
[a1.'"b"']
[a1.'"b"'.c]
answer = 42 

["key#group"]
answer = 42

        )";

        auto root = toml::loads(context);

        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 42, "a1", R"("b")", "c", "answer"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 42, "key#group", "answer"));

        CHECK(CheckTomlEmptyTable(root, "a", "b", "c"));
        CHECK(CheckTomlEmptyTable(root, "a", "b.c"));
        CHECK(CheckTomlEmptyTable(root, "a", "d.e"));
        CHECK(CheckTomlEmptyTable(root, "a", " x "));
        CHECK(CheckTomlEmptyTable(root, "d", "e", "f"));
        CHECK(CheckTomlEmptyTable(root, "g", "h", "i"));
        CHECK(CheckTomlEmptyTable(root, "j", "ʞ", "l"));
        CHECK(CheckTomlEmptyTable(root, "x", "1", "2"));
        
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 1, "aa", "key"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 2, "aa", "extend", "key"));
        CHECK(LoadTomlValueAndExtract<toml::integer>(root, 3, "aa", "extend", "more", "key"));
    }
}

template <typename... Keys>
bool CheckTomlString(const toml::value& root, const char* result, Keys... keys)
{
    return LoadTomlValueAndExtract<toml::string>(root, result, keys...);
};

TEST_CASE("valid-string")
{
    SECTION("1")
    {
        constexpr const char* context = R"(
answer = ""
end_esc = "String does not end here\" but ends here\\"
lit_end_esc = 'String ends here\'

multiline_unicode = """
\u00a0"""

multiline_not_unicode = """
\\u0041"""

multiline_end_esc = """When will it end? \"""...""\" should be here\""""

lit_multiline_not_unicode = '''
\u007f'''

lit_multiline_end = '''There is no escape\'''

backspace     = "|\b."
tab           = "|\t."

newline       = "|\n."
formfeed      = "|\f."
carriage      = "|\r."
quote         = "|\"."
backslash     = "|\\."
delete        = "|\u007F."
unitseparator = "|\u001F."

# # \u is escaped, so should NOT be interperted as a \u escape.
notunicode1   = "|\\u."
notunicode2   = "|\u005Cu."
notunicode3   = "|\\u0075."
notunicode4   = "|\\\u0075."

empty-1 = """"""

# # A newline immediately following the opening delimiter will be trimmed.
empty-2 = """
"""

# # \ at the end of line trims newlines as well; note that last \ is followed by
# # two spaces, which are ignored.
empty-3 = """\
    """
empty-4 = """\
   \
   \  
   """
0="""\
"""

        )";

        auto root = toml::loads(context);

        CHECK(CheckTomlString(root, "", "0"));
 
        CHECK(CheckTomlString(root, " ", "multiline_unicode"));
        CHECK(CheckTomlString(root, "\\u0041", "multiline_not_unicode"));
        CHECK(CheckTomlString(root, "When will it end? \"\"\"...\"\"\" should be here\"", "multiline_end_esc"));
        CHECK(CheckTomlString(root, "\\u007f", "lit_multiline_not_unicode"));
        CHECK(CheckTomlString(root, "There is no escape\\", "lit_multiline_end"));

        CHECK(CheckTomlString(root, "|\b.", "backspace"));
        CHECK(CheckTomlString(root, "|\t.", "tab"));
        CHECK(CheckTomlString(root, "|\n.", "newline"));
        CHECK(CheckTomlString(root, "|\f.", "formfeed"));
        CHECK(CheckTomlString(root, "|\r.", "carriage"));
        CHECK(CheckTomlString(root, "|\".", "quote"));
        CHECK(CheckTomlString(root, "|\\.", "backslash"));
        CHECK(CheckTomlString(root, "|.", "delete"));
        CHECK(CheckTomlString(root, "|\u001f.", "unitseparator"));

        CHECK(CheckTomlString(root, "|\\u.", "notunicode1"));
        CHECK(CheckTomlString(root, "|\\u.", "notunicode2"));
        CHECK(CheckTomlString(root, "|\\u0075.", "notunicode3"));
        CHECK(CheckTomlString(root, "|\\u.", "notunicode4"));
        
        CHECK(CheckTomlString(root, "", "answer"));

        CHECK(CheckTomlString(root, "", "empty-1"));
        CHECK(CheckTomlString(root, "", "empty-2"));
        CHECK(CheckTomlString(root, "", "empty-3"));
        CHECK(CheckTomlString(root, "", "empty-4"));

        CHECK(CheckTomlString(root, "String ends here\\", "lit_end_esc"));
        CHECK(CheckTomlString(root, "String does not end here\" but ends here\\", "end_esc"));

    }

    SECTION("2")
    {
        constexpr const char* context = R"(
# Make sure that quotes inside multiline strings are allowed, including right
# after the opening '''/""" and before the closing '''/"""

lit_one = ''''one quote''''
lit_two = '''''two quotes'''''
lit_one_space = ''' 'one quote' '''
lit_two_space = ''' ''two quotes'' '''

one = """"one quote""""
two = """""two quotes"""""
one_space = """ "one quote" """
two_space = """ ""two quotes"" """

mismatch1 = """aaa'''bbb"""
mismatch2 = '''aaa"""bbb'''

# Three opening """, then one escaped ", then two "" (allowed), and then three
# closing """
escaped = """lol\""""""

five-quotes = """
Closing with five quotes
"""""
four-quotes = """
Closing with four quotes
""""
        )";

        auto root = toml::loads(context);

        CHECK(CheckTomlString(root, "Closing with four quotes\n\"", "four-quotes"));
        CHECK(CheckTomlString(root, "Closing with five quotes\n\"\"", "five-quotes"));
        CHECK(CheckTomlString(root, "lol\"\"\"", "escaped"));

        CHECK(CheckTomlString(root, "aaa'''bbb", "mismatch1"));
        CHECK(CheckTomlString(root, "aaa\"\"\"bbb", "mismatch2"));
        
        CHECK(CheckTomlString(root, " \"\"two quotes\"\" ", "two_space"));
        CHECK(CheckTomlString(root, " \"one quote\" ", "one_space"));
        CHECK(CheckTomlString(root, "\"\"two quotes\"\"", "two"));
        CHECK(CheckTomlString(root, "\"one quote\"", "one"));

        CHECK(CheckTomlString(root, " ''two quotes'' ", "lit_two_space"));
        CHECK(CheckTomlString(root, " 'one quote' ", "lit_one_space"));
        CHECK(CheckTomlString(root, "''two quotes''", "lit_two"));
        CHECK(CheckTomlString(root, "'one quote'", "lit_one"));
    }
}

// https://github.com/toml-lang/toml-test/tree/master/tests

json::value ParseAsJsonValue(std::string context)
{
    auto parser = JsonDecoder(std::move(context));
    return parser.parse_value();
}

struct Index
{
    union 
    {
        std::string key;
        int index;
    };
    bool ik;

    bool IsIndex() const { return ik == false; }
    bool IsKey() const { return ik == true; }

    Index(std::string key) : key(key), ik(true) { }
    Index(int index) : index(index), ik(false) { }

    Index(const Index& other)
    {
        if (other.IsKey())
        {
            key = other.key;
        }
        else
        {
            index = other.index;
        }
        ik = other.ik;
    }

    ~Index()
    {
        if (IsKey())
        {
            key.~basic_string();
        }
    }

    std::string ToString() const
    {
        return IsKey() ? key : std::to_string(index);
    }
};

const json::value* SearchJsonTableOrArray(const json::value& root, const std::vector<Index>& indices)
{
    const json::value* cur = &root;

    for (const auto& idx : indices)
    {
        if (idx.IsKey())
            cur = &(cur->as<json::object>().find(idx.key)->second);
        else
            cur = &(cur->as<json::array>()[idx.index]);
    }
    return cur;
}

std::generator<std::string> RecurseFile(std::string dir)
{
    for (const auto& file : std::filesystem::directory_iterator(dir))
    {
        if (file.is_directory())
        {
            for (const auto& subfile : RecurseFile(file.path().string()))
            {
                co_yield subfile;
            }
        }
        else
        {
            co_yield file.path().string();
        }
    }
}

inline constexpr auto EndWith = [](const char* suffix) 
{
    return [=](const auto& entry) 
    {
        return entry.ends_with(suffix);
    };
};

inline constexpr auto RemoveExtension = [](const std::string& name)
{
    return name.substr(0, name.rfind('.'));
};

auto Read(const char* dir, const char* extension)
{
    return RecurseFile(dir) 
         | std::views::filter(EndWith(extension))
         | std::views::transform(RemoveExtension)
         | std::ranges::to<std::vector<std::string>>();
}

void DebugFile(const char* file)
{
    auto t = toml::load(file);
    auto j = cpp::config::toml2json()(t);
    Console::WriteLine(j);
}

const char* directory = R"(D:\code\toml-test\tests)";


