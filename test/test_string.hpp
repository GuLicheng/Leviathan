#pragma once


#include <iostream>
#include <functional>
#include <algorithm>
#include <string>
#include <string_view>
#include <ranges>
#include <string.h>

#include "base.hpp"


LV_TEST_BEGIN

template <typename StringType>
void test_string()
{
    STATIC_REQUIRE(std::ranges::contiguous_range<StringType>);

    constexpr char chars[] = "[Hello World !]";
    constexpr auto chars_len = ::strlen(chars);  // remove '\0'
    
    SECTION("Capacity")
    {
        StringType str = chars;
        REQUIRE(str.size() == chars_len);
        REQUIRE(!str.empty());
    }

    SECTION("Iterators")
    {
        StringType str = chars;
        REQUIRE(std::equal(str.data(), str.data() + str.size(), chars));
        REQUIRE(std::equal(str.begin(), str.end(), chars));
        std::string reversed_str{ str.rbegin(), str.rend() };
        REQUIRE(std::ranges::equal(reversed_str, str | std::views::reverse));
    }

    SECTION("StringView")
    {
        StringType str = chars;
        REQUIRE(std::string_view(chars) == str.sv()); 
    }

    SECTION("Element access")
    {
        StringType str = chars;
        REQUIRE(str.front() == '[');
        REQUIRE(str.back() == ']');
        REQUIRE(str[1] == 'H');
        REQUIRE(str.at(1) == 'H');
    }

    SECTION("Hash code")
    {
        StringType str = chars;
        std::hash<std::string_view> hasher;
        REQUIRE(str.hash_code() == hasher(std::string_view(chars)));
    }

}




LV_TEST_END


