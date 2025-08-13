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
    json::value root1 = json::object();

    root1["context1"] = { true, 1, "Hello", nullptr, { 1, 2, 3 }, { { "name", "Alice" }, { "age", 18 } } };
    root1["context2"]["array"] = { 1, 2, 3, 4, 5 };
    root1["context3"]["object"] = { { "key1", "value1" }, { "key2", 42 } };

    std::println("{:4}", root1);

    auto roo2 = cpp::cast<toml::value>(root1);  // TOML FIXME: BUG { "key2", 42 }

    std::println("root2 = \n{}", (roo2)); 

    return 0;
}