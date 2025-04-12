#include "value_cast.hpp"
#include "json/json.hpp"
#include "toml/toml.hpp"
#include <catch2/catch_all.hpp>

namespace json = cpp::json;
namespace toml = cpp::toml;

TEST_CASE("toml -> json")
{
    toml::value toml_root = toml::loads(R"(
        ok = true
    )");

    auto json_key = json::string("ok");
    auto json_root = cpp::config::toml2json()(toml_root);
    CHECK(json_root.as<json::object>()[json_key].as<json::boolean>() == true);
}

TEST_CASE("json -> toml")
{
    json::value json_root = json::loads(R"({ "ok": false })");

    auto toml_key = toml::string("ok");
    auto toml_root = cpp::config::json2toml()(json_root);
    CHECK(toml_root.as<toml::table>()[toml_key].as<toml::boolean>() == false);
}

