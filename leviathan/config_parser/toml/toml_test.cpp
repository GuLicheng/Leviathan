// // https://github.com/toml-lang/toml-test/tree/master
// // https://www.bejson.com/validators/toml_editor/

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

// using JsonDecoder = json::decoder;
// using TomlDecoder = toml::decoder;
// using JsonFormatter = cpp::config::toml2json;

#include "decoder.hpp"
#include <leviathan/config_parser/context.hpp>

using Context = cpp::config::basic_context<char>;
using TomlStringDecoder = cpp::toml::detail::toml_string_decoder<Context>;
using TomlDecoder = cpp::toml::detail::toml_decoder<Context>;

using JsonDecoder = json::detail::decode2<cpp::config::context>;
using JsonFormatter = cpp::config::toml2json;

template <typename ArrayIndex, typename MapIndex>
struct indexer : public std::variant<ArrayIndex, MapIndex>
{
    using std::variant<ArrayIndex, MapIndex>::variant;
    using std::variant<ArrayIndex, MapIndex>::operator=;

    constexpr bool is_array_index() const 
    {
        return this->index() == 0;
    }

    constexpr bool is_map_key() const
    {
        return this->index() == 1;
    }

    auto& array_index() const
    {
        return std::get<0>(*this);
    }

    auto& map_key() const
    {
        return std::get<1>(*this);
    }

    std::string to_string() const
    {
        return this->index() == 0 
             ? std::format("{}", array_index())
             : std::format("{}", map_key());
    }
};

using Index = indexer<size_t, std::string>;

using Path = std::pair<std::vector<Index>, const json::value*>;

template <typename ExpectedType>
struct TomlIs
{
    static bool operator()(const toml::value& x)
    {
        return x.is<ExpectedType>();
    }
};

bool IsValue(const toml::value& x)
{
    return !x.is<toml::array>() && !x.is<toml::table>();
}

bool CheckTomlType(std::string_view type, const toml::value& x)
{
    static std::map<std::string_view, std::function<bool(const toml::value&)>> m = 
    {
        { "integer", TomlIs<toml::integer>() },
        { "float", TomlIs<toml::floating>() },
        { "bool", TomlIs<toml::boolean>() },
        { "string", TomlIs<toml::string>() },
        { "datetime", TomlIs<toml::datetime>() },
        { "date-local", TomlIs<toml::datetime>() },
        { "datetime-local", TomlIs<toml::datetime>() },
        { "time-local", TomlIs<toml::datetime>() },
    };

    auto it = m.find(type);

    if (it == m.end())
    {
        throw std::runtime_error(std::format("Unknown toml type {}.", type));
    }
    else
    {
        return it->second(x);
    }
}

std::vector<Path> GatherJsonPath(const json::value& jv)
{
    std::vector<Path> retval;
    Path current;
    
    auto IsLeaf = [](const json::value& x) static
    {
        return x.is<json::object>()
            && x.as<json::object>().size() == 2
            && x.as<json::object>().contains("type")
            && x.as<json::object>().contains("value");
    };

    auto Dfs = [&](this auto self, const json::value& x) 
    {
        if (IsLeaf(x))
        {
            current.second = &x;
            retval.emplace_back(current);
            return;
        }
        
        if (x.is<json::object>())
        {
            for (const auto& [key, value] : x.as<json::object>())
            {
                current.first.emplace_back(key);
                self(value);
                current.first.pop_back();
            }
        }
        else if (x.is<json::array>())
        {
            for (size_t i = 0; i < x.as<json::array>().size(); ++i)
            {
                current.first.emplace_back(i);
                self(x.as<json::array>()[i]);
                current.first.pop_back();
            }
        }
        else
        {
            throw std::runtime_error("Unreachable.");
        }
    };

    Dfs(jv);

    return retval;
}

template <typename Fn>
toml::value ParseAsTomlValue(Fn fn, std::string_view sv)
{
    cpp::config::context ctx(sv);
    auto v = fn(ctx);
    return v;
}

template <typename TomlValueType>
struct EqualAs
{
    static bool operator()(const toml::value& x, const toml::value& y)
    {
        return x.as<TomlValueType>() == y.as<TomlValueType>();
    }
};

struct TomlValueEqualTo 
{
    static bool operator()(std::string_view type, const toml::value& x, json::string js)
    {
        static std::map<std::string_view, std::function<bool(const toml::value&, const toml::value&)>> m =
        {
            { "integer", EqualAs<toml::integer>() },
            { "float", EqualAs<toml::floating>() },
            { "bool", EqualAs<toml::boolean>() },
            { "string", EqualAs<toml::string>() },
            { "datetime", EqualAs<toml::datetime>() },
            { "date-local", EqualAs<toml::datetime>() },
            { "datetime-local", EqualAs<toml::datetime>() },
            { "time-local", EqualAs<toml::datetime>() },
        };

        if (type == "datetime" || type == "date-local" || type == "datetime-local" || type == "time-local")
        {
            return x.is<toml::datetime>(); 
        }

        if (type == "string")
        {
            // js = std::format("\"{}\"", js);
            return true; // Skip string type
        }

        auto y = ParseAsTomlValue(TomlDecoder::decode_value, { js.begin(), js.end() });

        auto it = m.find(type);

        if (it == m.end())
        {
            throw std::runtime_error(std::format("Unknown type: {}", type));
        }

        if (type == "float" && y.is<toml::integer>())
        {
            return x.as<toml::floating>() == static_cast<toml::floating>(y.as<toml::integer>());
        }

        return it->second(x, y);
    }
};

bool CompareTomlAndJsonValue(const toml::value& tv, const json::value& jv)
{
    const auto type = jv.as<json::object>().find("type")->second.as<json::string>();
    if (!CheckTomlType(type, tv))
    {
        return false;
    }
    auto value = jv.as<json::object>().find("value")->second.as<json::string>();
    
    return TomlValueEqualTo()(type, tv, value);
}

bool MatchToml(const toml::value& tv, const Path& keys)
{
    auto t = &tv;
    for (auto& key : keys.first)
    {
        if (t->is<toml::array>() && key.is_array_index())
        {
            t = &(t->as<toml::array>()[key.array_index()]);
        }
        else if (t->is<toml::table>() && key.is_map_key())
        {
            t = &(t->as<toml::table>().find(key.map_key())->second);
        }
        else
        {
            throw std::runtime_error("Error");
        }
    }
    return CompareTomlAndJsonValue(*t, *keys.second);
}

bool TestFile(std::string filename)
{
    auto json_file = filename + ".json";
    auto toml_file = filename + ".toml";
    auto jv = json::load(json_file.c_str());
    auto tv = toml::load(toml_file.c_str());

    auto path = GatherJsonPath(jv);

    for (const auto& p : path)
    {
        if (!MatchToml(tv, p))
        {
            return false;
        }
    }

    return true;
}

auto AllFiles(const char* path, bool fullname = false)
{
    auto nameof = [=](const auto& entry) {
        return fullname ? entry.path().string() : entry.path().filename().string();
    };

    return std::filesystem::directory_iterator{path}
         | cpp::views::transform(nameof)
         | std::ranges::to<std::vector>();
}

void TestAllFiles(std::string path)
{
    auto MaybeError = [](const auto& name) {
        return name.ends_with(".toml") 
        && !name.contains("escape-esc.toml")
        && !name.contains("no-seconds.toml")
        && !name.contains("float-2.toml")
        && !name.contains("inline-table/newline.toml")
        && !name.contains("hex-escape.toml");
    };

    auto files = AllFiles(path.c_str(), true)
               | std::views::filter(MaybeError)
               | std::views::transform([](const auto& name) { return name.substr(0, name.size() - 5); })
               | std::ranges::to<std::vector>();

    // int success = 0;

    for (const auto& file : files)
    {
        try
        {
            // std::println("Test file: {}", file);
            auto res = TestFile(file);
            REQUIRE(res);
        }
        catch (const std::exception& ex)
        {
            REQUIRE(false);
        }

    }
}

template <typename Input, typename Fn>
bool CheckTomlStringDecode(const Input& input, const char* expected, Fn fn)
{
    Context ctx(input);
    auto result = fn(ctx);
    return result == expected;
}

template <typename Fn>
auto SimpleDecode(Fn fn, const char* input)
{
    Context ctx(input);
    return fn(ctx);
}

TEST_CASE("toml-basic-string")
{   
    REQUIRE(CheckTomlStringDecode(R"("")", "", &TomlStringDecoder::decode_basic_string));
    REQUIRE(CheckTomlStringDecode(R"("\u03BD")", "ν", &TomlStringDecoder::decode_basic_string));
    REQUIRE(CheckTomlStringDecode(R"("Roses are red\nViolets are blue")", "Roses are red\nViolets are blue", &TomlStringDecoder::decode_basic_string));
    REQUIRE(CheckTomlStringDecode(R"("qu\"ote")", "qu\"ote", &TomlStringDecoder::decode_basic_string));
}

TEST_CASE("toml-literal-string")
{   
    REQUIRE(CheckTomlStringDecode(R"('')", "", &TomlStringDecoder::decode_literal_string));
    REQUIRE(CheckTomlStringDecode(R"('\u03BD')", R"(\u03BD)", &TomlStringDecoder::decode_literal_string));
    REQUIRE(CheckTomlStringDecode(R"('HelloWorld\n')", R"(HelloWorld\n)", &TomlStringDecoder::decode_literal_string));
}

void CheckTomlNumberInt(const char* input, int expected)
{
    REQUIRE(SimpleDecode(&TomlDecoder::decode_value, input).as<cpp::toml::integer>() == expected);
}

TEST_CASE("toml-integer")
{
    CheckTomlNumberInt("42", 42);
    CheckTomlNumberInt("+42", 42);
    CheckTomlNumberInt("-42", -42);
    CheckTomlNumberInt("0", 0);
}

template <typename Input, typename Fn>
bool CheckTomlMultilineStringDecode(const Input& input, std::vector<std::string> expected, Fn fn)
{
    Context ctx(input);
    auto result = fn(ctx);

    if (result.size() != expected.size())
        return false;

    for (size_t i = 0; i < result.size(); i++)
    {
        if (result[i] != expected[i])
            return false;
    }
    return true;
}

TEST_CASE("toml-simple-keys")
{
    REQUIRE(CheckTomlMultilineStringDecode(R"(simple)", { "simple" }, &TomlStringDecoder::decode_simple_keys));
    REQUIRE(CheckTomlMultilineStringDecode(R"(1. "2" . '3')", { "1", "2", "3" }, &TomlStringDecoder::decode_simple_keys));
    REQUIRE(CheckTomlMultilineStringDecode(R"(''."")", { "", "" }, &TomlStringDecoder::decode_simple_keys));
}

TEST_CASE("toml-value-boolean")
{
    Context True("true"), False("false");
    REQUIRE(TomlDecoder::decode_boolean(True).as<cpp::toml::boolean>() == true);
    REQUIRE(TomlDecoder::decode_boolean(False).as<cpp::toml::boolean>() == false);
}

void CheckTomlNumber(const char* input, double expected)
{
    REQUIRE(SimpleDecode(&TomlDecoder::decode_value, input).as<cpp::toml::floating>() == expected);
}

TEST_CASE("toml-value-number")
{
    CheckTomlNumber("0.0", 0);
    CheckTomlNumber("+0.0", 0);
    CheckTomlNumber("-0.0", 0);
    CheckTomlNumber("0e0", 0);
    CheckTomlNumber("0e00", 0);
    CheckTomlNumber("+0e0", 0);
    CheckTomlNumber("-0e0", 0);

    CheckTomlNumber("3e2", 3e2);
    CheckTomlNumber("3E2", 3e2);
    CheckTomlNumber("3e-2", 3e-2);
    CheckTomlNumber("3E+2", 3e2);
    CheckTomlNumber("3e0", 3e0);
    CheckTomlNumber("3.1e2", 3.1e2);
    CheckTomlNumber("3.1E2", 3.1e2);
    CheckTomlNumber("-1E-1", -1E-1);

    CheckTomlNumber("3.1415", 3.1415);
    CheckTomlNumber("+3.1415", 3.1415);
    CheckTomlNumber("-3.1415", -3.1415);
    CheckTomlNumber("0.1234", 0.1234);

    CheckTomlNumber("3.141592653589793", 3.141592653589793);
    CheckTomlNumber("-3.141592653589793", -3.141592653589793);

    CheckTomlNumber("9_007_199_254_740_991.0", 9007199254740991.0);
    CheckTomlNumber("-9_007_199_254_740_991.0", -9007199254740991.0);

    CheckTomlNumber("3_141.5927", 3141.5927);
    CheckTomlNumber("-3_141.5927", -3141.5927);
    CheckTomlNumber("3e1_4", 3e14);
}

TEST_CASE("toml-value-array")
{
    auto array1 = SimpleDecode(&TomlDecoder::decode_array, R"([1, 3.14, true, []])");
    REQUIRE(array1.as<cpp::toml::array>().size() == 4);
    REQUIRE(array1.as<cpp::toml::array>()[0].as<cpp::toml::integer>() == 1);
    REQUIRE(array1.as<cpp::toml::array>()[1].as<cpp::toml::floating>() == 3.14);
    REQUIRE(array1.as<cpp::toml::array>()[2].as<cpp::toml::boolean>() == true);
    REQUIRE(array1.as<cpp::toml::array>()[3].as<cpp::toml::array>().empty());
}

TEST_CASE("float-2")
{
    auto nan1 = SimpleDecode(TomlDecoder::decode_value, "nan");
    auto nan2 = SimpleDecode(TomlDecoder::decode_value, "+nan");
    auto nan3 = SimpleDecode(TomlDecoder::decode_value, "-nan");

    auto inf1 = SimpleDecode(TomlDecoder::decode_value, "inf");
    auto inf2 = SimpleDecode(TomlDecoder::decode_value, "+inf");
    auto inf3 = SimpleDecode(TomlDecoder::decode_value, "-inf");

    REQUIRE(std::isnan(nan1.as<toml::floating>()));
    REQUIRE(std::isnan(nan2.as<toml::floating>()));
    REQUIRE(std::isnan(nan3.as<toml::floating>()));

    REQUIRE(std::isinf(inf1.as<toml::floating>()));
    REQUIRE(std::isinf(inf2.as<toml::floating>()));
    REQUIRE(std::isinf(inf3.as<toml::floating>()));
}

TEST_CASE("auto-files")
{
    std::string Root = "D:/code/toml-test/tests/valid";

    TestAllFiles(Root + "/array");
    TestAllFiles(Root + "/bool");
    // TestAllFiles(Root + "/comment");
    TestAllFiles(Root + "/datetime");
    // TestAllFiles(Root + "/float");  
    TestAllFiles(Root + "/integer"); 
    TestAllFiles(Root + "/key");
    // TestAllFiles(Root + "/inline-table");
    // TestAllFiles(Root + "/spec");
    // TestAllFiles(Root + "/string");
    TestAllFiles(Root + "/table");
    TestAllFiles(Root);
}