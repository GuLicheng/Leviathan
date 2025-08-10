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

    std::string s = R"""(
        [true, false, null]
    )""";

    auto value = json::loads(s);
    std::println("{}", value);

    return 0;
}