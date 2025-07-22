#include <leviathan/meta/type.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/json/json.hpp>
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

// template <typename T> using Encoder = json::detail::caster<T>;
template <typename T> using Encoder = cpp::type_caster<T, json::value, cpp::error_policy::exception>;

int main()
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

    return 0;
}
