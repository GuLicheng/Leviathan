#include <variant>
#include <vector>
#include <ranges>
#include <string>
#include <functional>
#include <leviathan/print.hpp>

#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/value_cast.hpp>

namespace toml = leviathan::toml;
namespace json = leviathan::json;

template <typename ArrayIndex, typename MapIndex>
struct indexer : public std::variant<ArrayIndex, MapIndex>
{
    using std::variant<ArrayIndex, MapIndex>::variant;
    using std::variant<ArrayIndex, MapIndex>::operator=;

    constexpr bool is_array_index() const 
    {
        return this->index() == 0;
    }

    constexpr bool is_map_index() const
    {
        return this->index() == 1;
    }

    auto& array_index() const
    {
        return std::get<0>(*this);
    }

    auto& map_index() const
    {
        return std::get<1>(*this);
    }

    std::string to_string() const
    {
        return this->index() == 0 
             ? std::format("{}", array_index())
             : std::format("{}", map_index());
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

bool MatchToml(const toml::value& tv, const std::vector<Path>& keys)
{

}

void DebugFile(const char* file)
{
    auto t = toml::load(file);
    auto j = leviathan::config::detail::toml2json()(t);
    Console::WriteLine(j);
}


int main(int argc, char const *argv[])
{
    // DebugFile("../a.toml");
    system("chcp 65001");
    auto jv = json::load("../a.json");
    Console::WriteLine(jv);
    return 0;

    auto Indices = GatherJsonPath(jv);

    for (auto&& outer : Indices)
    {
        for (auto&& inner : outer.first)
        {
            Console::Write(inner.to_string());
            Console::Write(", ");
        }
        Console::WriteLine(*outer.second);
    }

    Console::WriteLine("====================================");

    return 0;
}

