#include <print>
#include <leviathan/meta/type.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <leviathan/math/int128.hpp>
#include <algorithm>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <functional>
#include <ranges>
#include <map>
#include <numeric>
#include <string>
#include <list>
#include <stack>

namespace json = cpp::config::json;
namespace toml = cpp::config::toml;

struct hack : std::stack<int> {
    using std::stack<int>::c;
};

int main()
{
    // std::bind_back()
    auto t = std::make_pair("123", "3.14");

    // auto f1 = cpp::make_tuple_callables(cpp::cast<int>, cpp::cast<double>)(std::make_pair("123", "3.14"));
    auto f1 = cpp::make_tuple_callables(cpp::cast<int>, cpp::cast<double>)(t);

    std::map<std::string, int> m = {
        { "Alice", 18 },  
        { "Bob", 17 },  
    };

    auto combine = [](std::string name, int age) { return std::format("name: {} and age: {}", name, age); };

    auto rg = m | cpp::views::apply(combine);
    // auto rg = std::apply(combine, *m.begin());

    std::println("{}", rg);
    return 0;
}