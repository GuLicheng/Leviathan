// https://github.com/toml-lang/toml-test/tree/master

#include "toml.hpp"
#include <catch2/catch_all.hpp>

#include <numeric>
#include <iostream>
#include <functional>

namespace toml = leviathan::toml;

TEST_CASE("toml_make")
{
    toml::value value1 = toml::make_toml<toml::integer>(3);
    toml::value value2 = toml::make_toml<toml::boolean>(true);
    toml::value value3 = toml::make_toml<toml::string>("HelloWorld");
    toml::value value4 = toml::make_toml<toml::array>();
    toml::value value5 = toml::make_toml<toml::table>();
    toml::value value6 = toml::make_toml<toml::floating>(3.14);
    // Datetime

    REQUIRE(value1.as<toml::integer>() == 3);
    REQUIRE(value2.as<toml::boolean>() == true);
    REQUIRE(value3.as<toml::string>() == "HelloWorld");
    REQUIRE(value4.as<toml::array>().empty());
    REQUIRE(value5.as<toml::table>().empty());
    REQUIRE(value6.as<toml::floating>() == 3.14);
}

bool CheckTableSize(const toml::value& root, size_t size)
{
    return root.is<toml::table>()
        && root.as<toml::table>().size() == size;
}

bool CheckArraySize(const toml::value& root, size_t size)
{
    return root.is<toml::array>()
        && root.as<toml::array>().size() == size;
}

template <typename TomlValueType, typename Expected>
bool TestSingleValue(const toml::value& root, const char* key, Expected expected)
{
    REQUIRE(root.is<toml::table>());
    auto it = root.as<toml::table>().find(key);
    return it != root.as<toml::table>().end()
        && it->second.as<TomlValueType>() == expected;
}

bool TestSingleValueIsNaNOrInf(const toml::value& root, const char* key, bool nan_or_inf)
{
    REQUIRE(root.is<toml::table>());
    auto it = root.as<toml::table>().find(key);
    return it != root.as<toml::table>().end()
        && (nan_or_inf 
            ? std::isnan(it->second.as<toml::floating>()) 
            : std::isinf(it->second.as<toml::floating>()));
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
const toml::value* AutomaticSearchTableOrArray(const toml::value& root, Key1 key1, Keys... keys)
{
    if constexpr (sizeof...(Keys) == 0)
    {
        return ExtractOneElementByIndex(root, key1);
    }
    else
    {
        const toml::value* next = ExtractOneElementByIndex(root, key1);
        return ExtractOneElementByIndex(*next, keys...);
    }
}

template <typename TomlValueType, typename Expected, typename... Keys>
bool ExtractTomlArrayOrTableElementAndCompare(const toml::value& root, Expected expected, Keys... keys)
{
    const toml::value* node = AutomaticSearchTableOrArray(root, keys...);
    return node->is<TomlValueType>() && node->as<TomlValueType>() == expected;
}

template <typename TomlValueType, typename Expected>
bool ExtractTomlArrayElementAndCompare(const toml::value& root, std::initializer_list<int> indices, Expected expected)
{
    const toml::value* cur = &root;
    for (const auto index : indices)
    {
        cur = &(cur->as<toml::array>()[index]);
    }
    return cur->is<TomlValueType>() && cur->as<TomlValueType>() == expected;
}

TEST_CASE("boolean")
{
    SECTION("valid")
    {
        constexpr const char* context = R"(
        
        t = true
        f = false
        
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 2));
        CHECK(TestSingleValue<toml::boolean>(root, "t", true));
        CHECK(TestSingleValue<toml::boolean>(root, "f", false));
    }
}

TEST_CASE("floating")
{
    SECTION("valid-zero")
    {
        constexpr const char* context = R"(
            zero = 0.0
            signed-pos = +0.0
            signed-neg = -0.0
            exponent = 0e0
            exponent-two-0 = 0e00
            exponent-signed-pos = +0e0
            exponent-signed-neg = -0e0
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 7));
        CHECK(TestSingleValue<toml::floating>(root, "zero", 0.0));
        CHECK(TestSingleValue<toml::floating>(root, "signed-pos", +0.0));
        CHECK(TestSingleValue<toml::floating>(root, "signed-neg", -0.0));
        CHECK(TestSingleValue<toml::floating>(root, "exponent", 0e0));
        CHECK(TestSingleValue<toml::floating>(root, "exponent-two-0", 0e00));
        CHECK(TestSingleValue<toml::floating>(root, "exponent-signed-pos", +0e0));
        CHECK(TestSingleValue<toml::floating>(root, "exponent-signed-neg", -0e0));
    }

    SECTION("valid-underscore")
    {
        constexpr const char* context = R"(
            before = 3_141.5927
            after = 3141.592_7
            exponent = 3e1_4
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 3));
        CHECK(TestSingleValue<toml::floating>(root, "before", 3'141.5927));
        CHECK(TestSingleValue<toml::floating>(root, "after", 3141.592'7));
        CHECK(TestSingleValue<toml::floating>(root, "exponent", 3e1'4));
    }

    SECTION("valid-max-int")
    {
        constexpr const char* context = R"(
            # Maximum and minimum safe natural numbers.
            max_float =  9_007_199_254_740_991.0
            min_float = -9_007_199_254_740_991.0
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 2));
        CHECK(TestSingleValue<toml::floating>(root, "max_float", 9'007'199'254'740'991.0));
        CHECK(TestSingleValue<toml::floating>(root, "min_float", -9'007'199'254'740'991.0));
    }

    SECTION("valid-long")
    {
        constexpr const char* context = R"(
            longpi = 3.141592653589793
            neglongpi = -3.141592653589793
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 2));
        CHECK(TestSingleValue<toml::floating>(root, "longpi", 3.141592653589793));
        CHECK(TestSingleValue<toml::floating>(root, "neglongpi", -3.141592653589793));
    }
    
    SECTION("valid-inf-and-nan")
    {
        constexpr const char* context = R"(
            # We don't encode +nan and -nan back with the signs; many languages don't
            # support a sign on NaN (it doesn't really make much sense).
            nan = nan
            nan_neg = -nan
            nan_plus = +nan
            infinity = inf
            infinity_neg = -inf
            infinity_plus = +inf
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 6));
        CHECK(TestSingleValueIsNaNOrInf(root, "nan", true));
        CHECK(TestSingleValueIsNaNOrInf(root, "nan_neg", true));
        CHECK(TestSingleValueIsNaNOrInf(root, "nan_plus", true));
        CHECK(TestSingleValueIsNaNOrInf(root, "infinity", false));
        CHECK(TestSingleValueIsNaNOrInf(root, "infinity_neg", false));
        CHECK(TestSingleValueIsNaNOrInf(root, "infinity_plus", false));
    }

    SECTION("valid-float")
    {
        constexpr const char* context = R"(
            pi = 3.14
            pospi = +3.14
            negpi = -3.14
            zero-intpart = 0.123
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 4));
        CHECK(TestSingleValue<toml::floating>(root, "pi", 3.14));
        CHECK(TestSingleValue<toml::floating>(root, "pospi", 3.14));
        CHECK(TestSingleValue<toml::floating>(root, "negpi", -3.14));
        CHECK(TestSingleValue<toml::floating>(root, "zero-intpart", 0.123));
    }

    SECTION("valid-exponent")
    {
        constexpr const char* context = R"(
            lower = 3e2
            upper = 3E2
            neg = 3e-2
            pos = 3E+2
            zero = 3e0
            pointlower = 3.1e2
            pointupper = 3.1E2
            minustenth = -1E-1
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 8));
        CHECK(TestSingleValue<toml::floating>(root, "lower", 3e2));
        CHECK(TestSingleValue<toml::floating>(root, "upper", 3E2));
        CHECK(TestSingleValue<toml::floating>(root, "neg", 3e-2));
        CHECK(TestSingleValue<toml::floating>(root, "pos", 3e+2));
        CHECK(TestSingleValue<toml::floating>(root, "zero", 3e0));
        CHECK(TestSingleValue<toml::floating>(root, "pointlower", 3.1e2));
        CHECK(TestSingleValue<toml::floating>(root, "pointupper", 3.1E2));
        CHECK(TestSingleValue<toml::floating>(root, "minustenth", -1E-1));
    }
}

TEST_CASE("integer")
{
    SECTION("valid-zero")
    {
        constexpr const char* context = R"(
            d1 = 0
            d2 = +0
            d3 = -0

            h1 = 0x0
            h2 = 0x00
            h3 = 0x00000

            o1 = 0o0
            a2 = 0o00
            a3 = 0o00000

            b1 = 0b0
            b2 = 0b00
            b3 = 0b00000
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 12));
        CHECK(TestSingleValue<toml::integer>(root, "d1", 0));
        CHECK(TestSingleValue<toml::integer>(root, "d2", 0));
        CHECK(TestSingleValue<toml::integer>(root, "d3", 0));
        CHECK(TestSingleValue<toml::integer>(root, "h1", 0));
        CHECK(TestSingleValue<toml::integer>(root, "h2", 0));
        CHECK(TestSingleValue<toml::integer>(root, "h3", 0));
        CHECK(TestSingleValue<toml::integer>(root, "o1", 0));
        CHECK(TestSingleValue<toml::integer>(root, "a2", 0));
        CHECK(TestSingleValue<toml::integer>(root, "a3", 0));
        CHECK(TestSingleValue<toml::integer>(root, "b1", 0));
        CHECK(TestSingleValue<toml::integer>(root, "b2", 0));
        CHECK(TestSingleValue<toml::integer>(root, "b3", 0));
    }

    SECTION("valid-underscore")
    {
        constexpr const char* context = R"(
            kilo = 1_000
            x = 1_1_1_1
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 2));
        CHECK(TestSingleValue<toml::integer>(root, "kilo", 1'000));
        CHECK(TestSingleValue<toml::integer>(root, "x", 1'1'1'1));
    }

    SECTION("valid-long")
    {
        constexpr const char* context = R"(
            # int64 "should" be supported, but is not mandatory. It's fine to skip this
            # test.
            int64-max     = 9223372036854775807
            int64-max-neg = -9223372036854775808
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 2));
        CHECK(TestSingleValue<toml::integer>(root, "int64-max", std::numeric_limits<int64_t>::max()));
        CHECK(TestSingleValue<toml::integer>(root, "int64-max-neg", std::numeric_limits<int64_t>::min()));
    }

    SECTION("valid-integer")
    {
        constexpr const char* context = R"(
            answer = 42
            posanswer = +42
            neganswer = -42
            zero = 0
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 4));
        CHECK(TestSingleValue<toml::integer>(root, "answer", 42));
        CHECK(TestSingleValue<toml::integer>(root, "posanswer", 42));
        CHECK(TestSingleValue<toml::integer>(root, "neganswer", -42));
        CHECK(TestSingleValue<toml::integer>(root, "zero", 0));
    }

    SECTION("valid-literals")
    {
        constexpr const char* context = R"(
            bin1 = 0b11010110
            bin2 = 0b1_0_1

            oct1 = 0o01234567
            oct2 = 0o755
            oct3 = 0o7_6_5

            hex1 = 0xDEADBEEF
            hex2 = 0xdeadbeef
            hex3 = 0xdead_beef
            hex4 = 0x00987
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 9));
        CHECK(TestSingleValue<toml::integer>(root, "bin1", 0b11010110));
        CHECK(TestSingleValue<toml::integer>(root, "bin2", 0b1'0'1));

        CHECK(TestSingleValue<toml::integer>(root, "oct1", 001234567));
        CHECK(TestSingleValue<toml::integer>(root, "oct2", 0755));
        CHECK(TestSingleValue<toml::integer>(root, "oct3", 0765));

        CHECK(TestSingleValue<toml::integer>(root, "hex1", 0xDEADBEEF));
        CHECK(TestSingleValue<toml::integer>(root, "hex2", 0xdeadbeef));
        CHECK(TestSingleValue<toml::integer>(root, "hex3", 0xdeadbeef));
        CHECK(TestSingleValue<toml::integer>(root, "hex4", 0x00987));
    }

    SECTION("valid-float64-max")
    {
        constexpr const char* context = R"(
            # Maximum and minimum safe float64 natural numbers. Mainly here for
            # -int-as-float.
            max_int =  9_007_199_254_740_991
            min_int = -9_007_199_254_740_991
        )";


        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 2));
        CHECK(TestSingleValue<toml::integer>(root, "max_int", 9'007'199'254'740'991));
        CHECK(TestSingleValue<toml::integer>(root, "min_int", -9'007'199'254'740'991));
    }
}

TEST_CASE("array")
{
    SECTION("array-subtables")
    {
        [[maybe_unused]] constexpr const char* context = R"(
            [[arr]]
            [arr.subtab]
            val=1

            [[arr]]
            [arr.subtab]
            val=2
        )";

        // auto root = toml::loads(context);
        // REQUIRE(CheckTableSize(root, 1));
    }

    SECTION("bool")
    {
        constexpr const char* context = R"(
            a = [true, false]
        )";

        auto root = toml::loads(context);
        auto& arr = root.as<toml::table>().find("a")->second;
        REQUIRE(CheckArraySize(arr, 2));
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::boolean>(arr, {0}, true));
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::boolean>(arr, {1}, false));
    }

    SECTION("empty")
    {
        constexpr const char* context = R"(
            thevoid = [[[[[]]]]]
        )";

        auto root = toml::loads(context);
        auto& arr1 = root.as<toml::table>().find("thevoid")->second;
        CHECK(arr1.is<toml::array>());
        auto& arr2 = arr1.as<toml::array>().front();
        CHECK(arr2.is<toml::array>());
        auto& arr3 = arr2.as<toml::array>().front();
        CHECK(arr3.is<toml::array>());
        auto& arr4 = arr3.as<toml::array>().front();
        CHECK(arr4.is<toml::array>());
        auto& arr5 = arr4.as<toml::array>().front();
        CHECK(arr5.is<toml::array>());
    }

    SECTION("hetergeneous")
    {
        constexpr const char* context = R"(
            mixed = [[1, 2], ["a", "b"], [1.1, 2.1]]
        )";

        auto root = toml::loads(context);
        auto& arr = root.as<toml::table>().find("mixed")->second;
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::integer>(arr, {0, 0}, 1));
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::integer>(arr, {0, 1}, 2));
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::string>(arr, {1, 0}, "a"));
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::string>(arr, {1, 1}, "b"));
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::floating>(arr, {2, 0}, 1.1));
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::floating>(arr, {2, 1}, 2.1));
    }

    SECTION("mixed-int-array")
    {
        constexpr const char* context = R"(
            arrays-and-ints =  [1, ["Arrays are not integers."]]
        )";

        auto root = toml::loads(context);
        auto& arr = root.as<toml::table>().find("arrays-and-ints")->second;
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::integer>(arr, {0}, 1));
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::string>(arr, {1, 0}, "Arrays are not integers."));
    }

    SECTION("mixed-int-float")
    {
        constexpr const char* context = R"(
            ints-and-floats = [1, 1.1]
        )";

        auto root = toml::loads(context);
        auto& arr = root.as<toml::table>().find("ints-and-floats")->second;
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::integer>(arr, {0}, 1));
        REQUIRE(ExtractTomlArrayElementAndCompare<toml::floating>(arr, {1}, 1.1));
    }

    SECTION("mixed-int-string")
    {
        constexpr const char* context = R"(
            strings-and-ints = ["hi", 42]
        )";

        auto root = toml::loads(context);
        auto& arr = root.as<toml::table>().find("strings-and-ints")->second;
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::string>(arr, "hi", 0));
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::integer>(arr, 42, 1));
    }

    SECTION("mixed-string-table")
    {
        constexpr const char* context = R"(
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

        auto root = toml::loads(context);
        auto& contributors = root.as<toml::table>().find("contributors")->second;
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::string>(contributors, "Foo Bar <foo@example.com>", 0));
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::string>(contributors, "Baz Qux", 1, "name"));
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::string>(contributors, "bazqux@example.com", 1, "email"));
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::string>(contributors, "https://example.com/bazqux", 1, "url"));
    
        auto& mixed = root.as<toml::table>().find("mixed")->second;
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::string>(mixed, "a", 0, "k"));
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::string>(mixed, "b", 1));
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::integer>(mixed, 1, 2));
    }

    SECTION("nested")
    {
        constexpr const char* context = R"(
            nest = [["a"], ["b"]]
        )";

        auto root = toml::loads(context);
        auto& nest = root.as<toml::table>().find("nest")->second;
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::string>(nest, "a", 0, 0));
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::string>(nest, "b", 1, 0));
    }

    SECTION("table-array-string-backslash")
    {
        constexpr const char* context = R"(
            foo = [ { bar="\"{{baz}}\""} ]
        )";

        auto root = toml::loads(context);
        auto& foo = root.as<toml::table>().find("foo")->second;
        REQUIRE(ExtractTomlArrayOrTableElementAndCompare<toml::string>(foo, "\"{{baz}}\"", 0, "bar"));
    }

    SECTION("nested-inline-table")
    {
        constexpr const char* context = R"(
            a = [ { b = {} } ]
        )";

        auto root = toml::loads(context);
        auto& a = root.as<toml::table>().find("a")->second;
        auto table_ptr = AutomaticSearchTableOrArray(a, 0, "b");
        REQUIRE(table_ptr->is<toml::table>());
        REQUIRE(table_ptr->as<toml::table>().empty());
    }
}

// TODO - everywhere
TEST_CASE("comment")
{
    SECTION("valid-after-literal-no-ws")
    {
        constexpr const char* context = R"(
            inf=inf#infinity
            nan=nan#not a number
            true=true#true
            false=false#false
        )";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 4));
        CHECK(TestSingleValueIsNaNOrInf(root, "inf", false));
        CHECK(TestSingleValueIsNaNOrInf(root, "nan", true));
        CHECK(TestSingleValue<toml::boolean>(root, "true", true));
        CHECK(TestSingleValue<toml::boolean>(root, "false", false));
    }

    SECTION("valid-at-eof")
    {
        constexpr const char* context = R"(
            # This is a full-line comment
            key = "value" # This is a comment at the end of a line)";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 1));
        CHECK(TestSingleValue<toml::string>(root, "key", "value"));
    }

    SECTION("valid-noeol")
    {
        constexpr const char* context = "# single comment without any eol characters";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 0));
    }

    SECTION("valid-nonascii")
    {
        constexpr const char* context = "# ~  ÿ ퟿  ￿ 𐀀 􏿿";

        auto root = toml::loads(context);
        REQUIRE(CheckTableSize(root, 0));
    }

    SECTION("valid-everywhere")
    {
        [[maybe_unused]] constexpr const char* context = R"(
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

        // TODO
    }

    SECTION("valid-tricky")
    {
        // TODO
    }
}

















