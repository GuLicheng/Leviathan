#include <ranges>
#include <algorithm>
#include <vector>
#include <leviathan/print.hpp>
#include <leviathan/ranges/concat.hpp>
#include <deque>
#include <numeric>
#include <unordered_map>
#include <leviathan/config_parser/toml/collector.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <leviathan/config_parser/json/encoder.hpp>

namespace toml = leviathan::toml;

int main(int argc, char const *argv[])
{
    toml::collector coll;

    coll.add_entry({"GameMode"}, toml::make_toml<toml::string>("Battle"));    

    coll.switch_to_std_table({"std-table"});
    coll.add_entry({"Hero", "Hp"}, toml::make_toml<toml::integer>(500));

    coll.switch_to_array_table({"array-table"});
    coll.add_entry({"Primary", "Damage"}, toml::make_toml<toml::integer>(100));
    coll.add_entry({"Secondary", "Damage"}, toml::make_toml<toml::integer>(150));
    coll.collect();

    auto global = coll.dispose();

    // auto caster = leviathan::config::detail::toml2json();
    auto caster = leviathan::config::value_cast<leviathan::json::value, toml::value>();

    auto json = caster(global);

    Console::WriteLine(json);
    Console::WriteLine("Ok");

    return 0;
}
