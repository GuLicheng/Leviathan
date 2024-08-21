// https://github.com/toml-lang/toml-test/tree/master

#include "toml.hpp"
#include "../json/json.hpp"
#include "../value_cast.hpp"

#include <catch2/catch_all.hpp>

#include <leviathan/print.hpp>
#include <numeric>
#include <iostream>
#include <functional>

namespace toml = leviathan::toml;
namespace json = leviathan::json;

using JsonDecoder = json::decoder;
using TomlDecoder = toml::decoder;
using JsonFormatter = leviathan::config::detail::toml2json;

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

TEST_CASE("valid-datetime")
{
    auto CheckTomlDatetime = [](const char* context,
        int year, int month, int day, 
        int hour = 0, int minute = 0, int second = 0, 
        int nanosecond = 0, int offset = 0) static
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
    };

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
const toml::value* SearchTableOrArray(const toml::value& root, Key1 key1, Keys... keys)
{
    if constexpr (sizeof...(Keys) == 0)
    {
        return ExtractOneElementByIndex(root, key1);
    }
    else
    {
        const toml::value* next = ExtractOneElementByIndex(root, key1);
        return SearchTableOrArray(*next, keys...);
    }
}

template <typename TomlValueType, typename Actual, typename... Keys>
bool LoadTomlValueAndExtract(const char* context, Actual actual, Keys... keys)
{
    auto tv = ParseAsTomlValue(context);
    const toml::value* node = SearchTableOrArray(tv, keys...);
    return node->is<TomlValueType>() && node->as<TomlValueType>() == actual;
}

template <typename TomlValueType, typename Actual, typename... Keys>
bool LoadTomlValueAndExtract(const toml::value& root, Actual actual, Keys... keys)
{
    const toml::value* node = SearchTableOrArray(root, keys...);
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
    auto t = SearchTableOrArray(root, 0, "b");
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
    auto nested_array = SearchTableOrArray(root, "nested-array", 0, 0, 0, 0);
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

TEST_CASE("valid-table")
{

}

// https://github.com/toml-lang/toml-test/tree/master/tests
const char* directory = R"(D:\code\toml-test\tests)";

json::value ParseAsJsonValue(std::string context)
{
    auto parser = JsonDecoder(std::move(context));
    return parser.parse_value();
}

template <typename ExpectedType>
struct TomlIs
{
    static bool operator()(const toml::value& x)
    {
        return x.is<ExpectedType>();
    }
};

bool CheckTomlType(std::string_view type, const toml::value& x)
{
    static std::map<std::string_view, std::function<bool(const toml::value&)>> m = 
    {
        { "integer", TomlIs<toml::integer>() },
        { "bool", TomlIs<toml::boolean>() },
        { "string", TomlIs<toml::string>() },
        { "datetime", TomlIs<toml::datetime>() },
        { "float", TomlIs<toml::floating>() },
    };

    auto it = m.find(type);

    if (it == m.end())
    {
        throw std::runtime_error("Unknown toml type.");
    }
    else
    {
        return it->second(x);
    }
}

// For Toml: x = 1
// For Json: x : { type: "integer", "value": "1" }
bool CompareJsonAndTomlValue(const json::value& jval, const toml::value& tval);

struct JsonMatcher
{
    bool Match(const json::value& root, const std::vector<std::string>& keys, const toml::value& target)
    {
        const json::value* cur = &root;

        for (const auto& key : keys)
        {
            cur = &(cur->as<json::object>().find(key)->second);
        }

        return CompareJsonAndTomlValue(*cur, target);
    }
};

struct TomlExtractor
{
    using path = std::vector<std::string>;
    using value = toml::value;

    std::vector<std::pair<path, const value*>> paths;

    void Dfs(const toml::value& x, path& cur)
    {
        if (!x.is<toml::table>() || (x.is<toml::table>() && x.as<toml::table>().empty()))
        {
            paths.emplace_back(cur, &x);
            return;
        }

        assert(x.is<toml::table>());
        for (auto& t = x.as<toml::table>(); const auto& [key, value] : t)
        {
            cur.emplace_back(key);
            Dfs(value, cur);
            cur.pop_back();
        }
    }

public:

    void Extract(const toml::value& x)
    {
        // Console::WriteLine(leviathan::config::detail::toml2json()(x));
        path p;
        Dfs(x, p);
    }

    void CompareToJson(const json::value& root, std::string filename)
    {
        JsonMatcher matcher;

        for (const auto& path_v : paths)
        {
            const auto& [keys, val] = path_v;
            if (!matcher.Match(root, keys, *val))
            {
                Console::WriteLine("Error file: {}", filename);
            }
        }
    }
};

void DebugFile(const char* file)
{
    auto t = toml::load(file);
    auto j = leviathan::config::detail::toml2json()(t);
    Console::WriteLine(j);
}

int mai1n()
{

    DebugFile("../a.toml");

    return 0;
}
