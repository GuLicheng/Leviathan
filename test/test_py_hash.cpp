#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>

#include <lv_cpp/collections/internal/py_hash.hpp>
#include <lv_cpp/struct.hpp>
#include <lv_cpp/record_allocator.hpp>
#include <lv_cpp/fancy_ptr.hpp>

#include <algorithm>
#include <string>

using HashT = ::leviathan::collections::hash_set<int>;


TEST_CASE("element destroy", "[dtor]")
{

    using Int = Int32<false>;

    {
        ::leviathan::collections::hash_set<Int, Int::HashType> h;

        // rehash
        for (int i = 0; i < 10; ++i)
        {
            h.insert(Int(i));
            h.emplace(i);
        }
    }

    auto a = Int32<>::total_construct();
    auto b = Int32<>::total_destruct();

    REQUIRE(a == b);
}

TEST_CASE("exception thrown in constructor", "[emplace][exception]")
{
    
    {
        using Int = CopyThrowExceptionInt<false, 2>;
        leviathan::collections::hash_set<
            Int, 
            Int::HashType> h;

        // REQUIRE_THROWS(h.emplace());
        h.emplace();
    }

    auto a = Int32<>::total_construct();
    auto b = Int32<>::total_destruct();

    REQUIRE(a == b);
    REQUIRE(a != 0);

}


#include "test_random_int.hpp"

TEST_CASE("data structure is correct", "[insert][contains][erase]")
{
    ::leviathan::test::test_set_is_correct<HashT, false>();
}












