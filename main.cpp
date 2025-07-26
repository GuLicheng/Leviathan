#include <leviathan/meta/type.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <print>
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

// template <typename T> using Encoder = json::detail::caster<T>;
template <typename T> using Encoder = cpp::type_caster<T, json::value, cpp::error_policy::exception>;

auto Json()
{
    Encoder<std::string> encoder;

    json::value v = json::make_json<json::object>();

    v.as<json::object>().emplace("name", json::make_json<json::string>("John Doe"));
    v.as<json::object>().emplace("age", json::make_json<json::number>(18));
    v.as<json::object>().emplace("married", json::make_json<json::boolean>(false));

    auto result = encoder(v);

    std::print("Encoded JSON: {}\n", result);

    Encoder<std::set<double>> vectorEncoder;
    json::value arrayValue = json::make_json<json::array>();
    arrayValue.as<json::array>().emplace_back(json::make_json<json::string>("3.14"));
    arrayValue.as<json::array>().emplace_back(json::make_json<json::number>(20));
    arrayValue.as<json::array>().emplace_back(json::make_json<json::number>(30));
    auto vectorResult = vectorEncoder(arrayValue);
    std::print("Encoded Vector: {}\n", vectorResult);


    v.as<json::object>().clear();
    v.as<json::object>().emplace("salary", json::make_json<json::number>(1000));
    v.as<json::object>().emplace("bonus", json::make_json<json::number>(2000));
    
    // auto salary = Encoder<std::unordered_map<std::string, int>>()(v);
    auto salary = Encoder<std::vector<std::pair<std::string, int>>>()(v);
    std::print("Encoded Salary: {}. \n", salary);
    return v;
}

auto Toml()
{
    cpp::config::toml::value v = cpp::config::toml::make_toml<cpp::config::toml::table>();

    v.as<cpp::config::toml::table>().emplace("lang", cpp::config::toml::make_toml<cpp::config::toml::string>("toml"));
    v.as<cpp::config::toml::table>().emplace("name", cpp::config::toml::make_toml<cpp::config::toml::string>("John Doe"));
    v.as<cpp::config::toml::table>().emplace("age", cpp::config::toml::make_toml<cpp::config::toml::integer>(18));
    v.as<cpp::config::toml::table>().emplace("married", cpp::config::toml::make_toml<cpp::config::toml::boolean>(false));

    auto result = cpp::type_caster<std::string, cpp::config::toml::value, cpp::error_policy::exception>()(v);

    std::print("Encoded TOML: {}\n", result);

    cpp::type_caster<std::vector<double>, cpp::config::toml::value, cpp::error_policy::exception> vectorEncoder;
    cpp::config::toml::value arrayValue = cpp::config::toml::make_toml<cpp::config::toml::array>();
    arrayValue.as<cpp::config::toml::array>().emplace_back(cpp::config::toml::make_toml<cpp::config::toml::floating>(3.14));
    arrayValue.as<cpp::config::toml::array>().emplace_back(cpp::config::toml::make_toml<cpp::config::toml::integer>(20));
    arrayValue.as<cpp::config::toml::array>().emplace_back(cpp::config::toml::make_toml<cpp::config::toml::integer>(30));
    auto vectorResult = vectorEncoder(arrayValue);
    std::print("Encoded Vector: {}\n", vectorResult);
    return v;
}

int main()
{

    auto js = Json();
    auto tl = Toml();

    std::println("json: {}", cpp::cast<std::string>(cpp::cast<toml::value>(js)));
    std::println("toml: {}", cpp::cast<json::value>(tl));
    std::println("\n==============\n");
    return 0;
}
