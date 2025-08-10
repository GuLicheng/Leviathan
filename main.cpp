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

    cpp::json::value obj = {
        {"pi", 3.141},
        {"happy", true},
        {"name", std::string("Niels")},
        {"nothing", nullptr},
        {"answer", {{"everything", 42}}},
        {"list", {1, 0, 2, "json"}},
        {"object", {{"currency", "USD"}, {"value", 42.99}}}};

    // std::println("{:i8}", obj);

    // cpp::json::value arr = {1, 3.14, nullptr, "Hello", true, {-1}, {{"Alice", 18}}};
    // std::println("{}", arr);

    std::unordered_map<int, double> m = {   
        { 1, 3.14 },
        { 2, 2.17 }
    };

    cpp::json::value v = { 
        {"pi", 3.141},
        {"nature-e", 2.718},
    };

    std::println("{:I8}", obj);

    std::println("{}", std::set{ 1, 2, 3 });

    return 0;
}