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
#include <list>
#include <functional>
#include <ranges>
#include <string>
#include <list>

namespace json = cpp::config::json;
namespace toml = cpp::config::toml;

struct Int
{
    int m_value;

    template <std::floating_point F>
    constexpr explicit operator F(this Int) 
    {
        return static_cast<F>(1);
    }
};

int main()
{
    Int a;
    a.m_value = 3;

    auto c = cpp::cast<double>(a);

    std::println("{}", c);

    constexpr auto _ = cpp::math::div(10, 3);

    return 0;
}
