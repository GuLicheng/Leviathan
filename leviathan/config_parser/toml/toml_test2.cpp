#include <variant>
#include <print>
#include <vector>
#include <algorithm>
#include <ranges>
#include <string>
#include <iostream>
#include <functional>
#include <leviathan/print.hpp>

#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/value_cast.hpp>

namespace toml = cpp::toml;
namespace json = cpp::json;

using JsonDecoder = json::detail::decode2<cpp::config::context>;
using TomlDecoder = toml::detail::toml_decoder<cpp::config::context>;
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

toml::value ParseAsTomlValue(const char* context)
{
    auto sv = std::string_view(context);
    cpp::config::context ctx(sv);
    return TomlDecoder::decode_value(ctx);
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
    static bool operator()(std::string_view type, const toml::value& x, const toml::value& y)
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

        auto it = m.find(type);
        if (it == m.end())
        {
            throw std::runtime_error(std::format("Unknown type: {}", type));
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
    
    if (type == "string")
    {
        value = std::format("\"{}\"", value);
    }
    
    auto j2t = ParseAsTomlValue(value.c_str());
    return TomlValueEqualTo()(type, tv, j2t);
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

    return std::filesystem::recursive_directory_iterator{path}
         | cpp::views::transform(nameof)
         | std::ranges::to<std::vector>();
}

void TestAllFiles()
{
    auto MaybeError = [](const auto& name) {
        return name.ends_with(".toml") 
        && !name.contains("escape-esc.toml")
        && !name.contains("no-seconds.toml")
        && !name.contains("inline-table\\newline.toml")
        && !name.contains("hex-escape.toml");
    };

    auto files = AllFiles(R"(D:\code\toml-test\tests\valid)", true)
               | std::views::filter(MaybeError)
               | std::views::transform([](const auto& name) { return name.substr(0, name.size() - 5); })
               | std::ranges::to<std::vector>();

    int success = 0;

    for (const auto& file : files)
    {
        try
        {
            std::println("Test file: {}", file);
            success += TestFile(file);
        }
        catch (const std::exception& ex)
        {
            std::println("File {} error: {}", file, ex.what());
        }
    }

    std::println("Test {} files, {} success, {} failed.", files.size(), success, files.size() - success);
}

int main(int argc, char const *argv[])
{
    TestAllFiles();

    return 0;
}

