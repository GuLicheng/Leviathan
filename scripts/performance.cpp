#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <leviathan/extc++/all.hpp>
#include <map>
#include <print>

constexpr const char* Root = R"(F:/CCB/Work/Performance/details)";

using Details = std::map<std::string, double>;
using EmployeeDetails = std::map<std::string, Details>;

template <typename AssociateContainer>
inline constexpr cpp::ranges::adaptor CollectAs = []<typename... Args>(Args&&... args) static
{
    auto fn = []<typename Tuple, typename R>(Tuple&& t, R&& r) static
    {
        auto retval = std::make_from_tuple<AssociateContainer>((Tuple&&)t);

        for (const auto& [name, details] : r)
        {
            auto& target_details = retval[name];
            for (const auto& [item, score] : details)
            {
                target_details[item] += score;
            }
        }
        return retval;
    };

    return cpp::ranges::partial<decltype(fn), std::decay_t<Args>...>(std::move(fn), (Args&&)args...);
};

int main(int argc, char const *argv[])
{
    system("chcp 65001"); // Set console to UTF-8 encoding

    // Cast value to EmployeeDetails may not efficient but more readable and clear
    auto rg = cpp::listdir(Root, true)
            | cpp::views::compose(cpp::toml::load, cpp::cast<EmployeeDetails>)
            | cpp::views::join
            | CollectAs<EmployeeDetails>();

    std::println("{}", rg["08193520"]);

    return 0;
}

