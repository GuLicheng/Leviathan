#include <leviathan/config_parser/toml.hpp>
#include <catch2/catch_all.hpp>


TEST_CASE("test split key")
{
    auto check = [](std::string_view key, auto... svs) {
        
        auto res = leviathan::config::toml::detail::split_keys(key);

        std::string_view keys[] = { svs... };

        REQUIRE(std::ranges::equal(res, keys));
    };

    SECTION("invalid input")
    {
        check(R"(name = "Orange")", "name");
        check(R"(physical.color = "orange")", "physical", "color");
        check(R"(site."google.com" = true)", "site", "google.com");
        check(R"("" = "blank")", "");
        check(R"('' = "blank")", "");
        check(R"("127.0.0.1" = "value")", "127.0.0.1");
        check(R"("character encoding" = "value")", "character encoding");
        check(R"("ʎǝʞ" = "value")", "ʎǝʞ");
        check(R"('key2' = "value")", "key2");
        check(R"('quoted "value"' = "value")", "quoted \"value\"");
        check(R"(key = "value"")", "key");
        check(R"('key2' = "value")", "key2");
        check(R"('bare_key' = "value")", "bare_key");
        check(R"('bare-key' = "value")", "bare-key");
        check(R"(1234 = "value")", "1234");
        check(R"(fruit.name = "value")", "fruit", "name");
        check(R"(fruit. name = "value")", "fruit", "name");
        check(R"(fruit . name = "value")", "fruit", "name");
    }

    SECTION("invalid input")
    {
        // std::string_view s[] = {
        //     R"(= "no key name")", 
        // };

        // REQUIRE_THROWS()
    }
}
