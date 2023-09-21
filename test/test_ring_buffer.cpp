#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <iostream>
#include <string>
#include <vector>
#include <format>
#include <list>
#include <random>
#include <algorithm>

#include <leviathan/collections/internal/ring_buffer.hpp>
// #include <catch2/catch_all.hpp>
#include <catch2/catch_all.hpp>


TEST_CASE("emplace front and back")
{
    leviathan::collections::ring_buffer<int> buffer;

    buffer.emplace_front(-1);
    buffer.emplace_front(-2);

    buffer.emplace_back(1);
    buffer.emplace_back(2);

    REQUIRE(buffer.size() == 4);
    REQUIRE(buffer.capacity() >= 4);

    REQUIRE(buffer[0] == -2);
    REQUIRE(buffer[1] == -1);
    REQUIRE(buffer[2] == 1);
    REQUIRE(buffer[3] == 2);

}

