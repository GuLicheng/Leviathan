#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <leviathan/extc++/all.hpp>
#include <unordered_map>
#include <map>
#include <set>
#include <leviathan/meta/type.hpp>
#include <leviathan/print.hpp>
#include <print>
#include <filesystem>
#include <iostream>

constexpr const char* Root = R"(F:/CCB/Work/Performance/details)";

using Details = std::unordered_map<std::string, double>;

struct MapMerger
{
    template <typename M1, typename M2>
    auto void operator()(M1& m1, const M2& m2) const
    {
        for (const auto& [key, value] : m2)
        {
            auto [pos, ok] = map.try_emplace(key, value);

            if (!ok)
            {
                pos->second += value; // Assuming the values are numeric and can be summed
            }
        }
    }
};

template <typename AssociateContainer>
inline constexpr adaptor collect = []<typename... Args>(Args&&... args) static
{
    auto fn = []<typename Tuple, typename R>(Tuple&& t, R&& r) static
    {
        auto map = std::make_from_tuple<AssociateContainer>((Tuple&&)t);

        for (auto&& [key, value] : r)
        {
            // The multimap will not provide `try_emplace`, it will cause compiler error.
            auto [pos, ok] = map.try_emplace(key, value);

            if (!ok)
            {
                MapMerger()(pos->second, value);
            }
        }

        return map;
    };

    return partial<decltype(fn), std::decay_t<Args>...>(std::move(fn), (Args&&)args...);
};

int main(int argc, char const *argv[])
{
    system("chcp 65001"); // Set console to UTF-8 encoding


    auto rg = cpp::listdir(Root, true)
            | cpp::views::compose(cpp::toml::load, cpp::cast<std::map<std::string, Details>>)
            | cpp::views::join
            | std::ranges::to<std::multimap>()
            | cpp::views::chunk_by(std::ranges::equal_to())
            | cpp::views::transform([](auto&& subrange) static { return subrange.begin()->first; })
            | std::ranges::to<std::set>()
            ;


    Console::WriteLine(rg);

    return 0;
}

