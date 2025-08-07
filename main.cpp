#include <print>
#include <leviathan/meta/type.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <leviathan/math/int128.hpp>
#include <set>
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
#include <leviathan/collections/container_interface.hpp>

namespace json = cpp::config::json;
namespace toml = cpp::config::toml;

int main()
{
    std::multiset<int> vec = {1, 2, 3, 4, 5, 2, 1, 2, 3};

    std::unordered_multimap<std::string, int> map = {
        {"Hello", 1},
        {"World", 2},
        {"Hello", 3},
    };

    // std::

    auto rg = vec | cpp::ranges::counter<std::map<int, int>>();

    std::print("Counter Map: {}\n", rg);



    return 0;
}
