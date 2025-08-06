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

template <typename AssociateContainer, typename... Args>
struct collect_fn : std::ranges::range_adaptor_closure<collect_fn<AssociateContainer, Args...>>
{
    std::tuple<Args...> m_args;

    template <typename... Ts>
    collect_fn(Ts&&... ts) : m_args((Ts&&) ts...) { }
 
    template <typename Self, std::ranges::viewable_range R>
    constexpr auto operator()(this Self&& self, R&& r)
    {
        auto map = std::make_from_tuple<AssociateContainer>(((Self&&)self).m_args);
        
        for (auto&& [key, value] : r)
        {
            auto [pos, ok] = map.try_emplace(key, value);

            if (!ok)
            {
                pos->second += value;
            }
        }

        return map;
    }
};

template <typename AssociateContainer>
inline constexpr cpp::ranges::adaptor collect = 
    []<typename... Args>(Args&&... args) static
{
    return collect_fn<AssociateContainer, std::decay_t<Args>...>((Args&&)args...);
};

template <typename AssociateContainer, typename... Args>
struct counter_fn : std::ranges::range_adaptor_closure<counter_fn<AssociateContainer, Args...>>
{
    std::tuple<Args...> m_args;

    template <typename... Ts>
    counter_fn(Ts&&... ts) : m_args((Ts&&) ts...) { }
 
    template <typename Self, std::ranges::viewable_range R>
    constexpr auto operator()(this Self&& self, R&& r)
    {
        using MappedType = typename AssociateContainer::mapped_type;
        static_assert(std::integral<MappedType>);

        auto map = std::make_from_tuple<AssociateContainer>(((Self&&)self).m_args);
        
        for (auto&& value : r)
        {
            auto [pos, ok] = map.try_emplace(value, (MappedType)1);

            if (!ok)
            {
                pos->second++;
            }
        }

        return map;
    }
};

template <typename AssociateContainer>
inline constexpr cpp::ranges::adaptor counter = 
    []<typename... Args>(Args&&... args) static
{
    return counter_fn<AssociateContainer, std::decay_t<Args>...>((Args&&)args...);
};

int main()
{
    std::multiset<int> vec = {1, 2, 3, 4, 5, 2, 1, 2, 3};

    std::unordered_multimap<std::string, int> map = {
        {"Hello", 1},
        {"World", 2},
        {"Hello", 3},
    };

    auto rg = vec | counter<std::map<int, int>>();

    std::print("Counter Map: {}\n", rg);

    return 0;
}
