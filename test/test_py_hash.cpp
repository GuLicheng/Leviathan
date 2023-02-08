#define CATCH_CONFIG_MAIN

#include <thirdpart/catch.hpp>

#include <lv_cpp/collections/py_hash.hpp>
#include <algorithm>
#include <string>

using HashT = ::python::collections::hash_set<int>;

#include "struct.hpp"
#include "except_allocator.hpp"

TEST_CASE("element destroy", "[dtor]")
{

    {
        ::python::collections::hash_set<Int32<>> h;

        // rehash
        for (int i = 0; i < 10; ++i)
        {
            h.insert(Int32<>(i));
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
        using Int = Int32<>;
        python::collections::hash_set<
            Int, 
            std::hash<Int>, 
            std::equal_to<Int>, 
            ExceptionAllocator<Int>> h;

        // REQUIRE_THROWS(h.emplace());
        h.emplace();
    }

    auto a = Int32<>::total_construct();
    auto b = Int32<>::total_destruct();

    REQUIRE(a == b);
    REQUIRE(a != 0);

}

TEST_CASE("no construable")
{
    // NoDefaultConstructable _;
    ::python::collections::hash_set<NoDefaultConstructable> hs;
    hs.emplace(1);
    REQUIRE(hs.size() == 1);
}












