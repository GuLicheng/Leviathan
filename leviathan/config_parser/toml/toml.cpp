#include "value.hpp"
#include "encoder.hpp"
#include "toml.hpp"
#include <leviathan/print.hpp>

namespace leviathan::config::toml
{
    
bool IsLeaf(const value& x)
{
    return x.data().index() <= 4
        || (x.is<array>() && x.as<array>().is_locked())
        || (x.is<table>() && x.as<table>().is_locked());
}

struct TomlEncoder
{
    enum ValueKind : bool { Array, Table };

    void operator()(const value& x, std::vector<std::string_view>& super, ValueKind kind = Table)
    {
        std::vector<std::pair<std::string_view, const value*>> leafs;
        std::vector<std::pair<std::string_view, const value*>> subtables;
        std::vector<std::pair<std::string_view, const value*>> subarrays;

        for (const auto& [k, v] : x.as<table>())
        {
            if (IsLeaf(v))
            {
                leafs.emplace_back(k, &v);
            }
            else if (v.is<table>())
            {
                subtables.emplace_back(k, &v);
            }
            else
            {
                // array
                subarrays.emplace_back(k, &v);
            }
        } 
    
        Console::WriteLine();

        // Try write section
        if (super.size())
        {
            const char* fmt = kind == Table ? "[{}]" : "[[{}]]";
            Console::WriteLine(fmt, super | std::views::join_with('.') | std::ranges::to<std::string>());
        }

        for (auto& leaf : leafs)
        {
            Console::WriteLine("{} = {}", leaf.first, encoder()(*leaf.second));
        }

        for (auto& subtable : subtables)
        {
            super.emplace_back(subtable.first);
            operator()(*subtable.second, super);
            super.pop_back();
        }

        for (auto& subarray : subarrays)
        {
            for (auto& arr : subarray.second->as<array>())
            {
                super.emplace_back(subarray.first);
                operator()(arr, super, Array);
                super.pop_back();
            }
        }
    }
};

} // namespace leviathan::config::toml

namespace toml = leviathan::toml;

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

using Path = std::pair<std::vector<Index>, const toml::value*>;

std::vector<Path> GatherTomlPath(const toml::value& jv)
{
    std::vector<Path> retval;
    Path current;
    
    auto Dfs = [&](this auto self, const toml::value& x) 
    {
        if (toml::IsLeaf(x))
        {
            current.second = &x;
            retval.emplace_back(current);
            return;
        }
        
        if (x.is<toml::table>())
        {
            for (const auto& [key, value] : x.as<toml::table>())
            {
                current.first.emplace_back(key);
                self(value);
                current.first.pop_back();
            }
        }
        else if (x.is<toml::array>())
        {
            for (size_t i = 0; i < x.as<toml::array>().size(); ++i)
            {
                current.first.emplace_back(i);
                self(x.as<toml::array>()[i]);
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

int main(int argc, char const *argv[])
{
    auto tv = toml::load("../a.toml");

    auto s = toml::dump(tv);

    Console::WriteLine("{}\n", s, "OK");
    return 0;
}

