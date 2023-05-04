#include <utils/layout.hpp>

#include <catch2/catch_all.hpp>

#include <tuple>
#include <bitset>
#include <iostream>
#include <cstddef>
#include <array>
#include <algorithm>
#include <bit>
#include <span>
#include <vector>

using namespace leviathan;

TEST_CASE("test_layout")
{
    static_assert(layout<int32_t>(3).offset<0>() == 0);
    static_assert(layout<int32_t>(3).offsets() == std::array<size_t, 1>{ 0 });
    static_assert(layout<int32_t>(3).alloc_size() == 12);
    static_assert(layout<int32_t>(3).size<0>() == 3);

    static_assert(layout<int32_t, int32_t>(3, 5).offset<0>() == 0);
    static_assert(layout<int32_t, int32_t>(3, 5).offset<1>() == 12);
    static_assert(layout<int32_t, int32_t>(3, 5).offsets() == std::array<size_t, 2>{ 0, 12 });
    static_assert(layout<int32_t, int32_t>(3, 5).alloc_size() == 32);
    static_assert(layout<int32_t, int32_t>(3, 5).size<0>() == 3);
    static_assert(layout<int32_t, int32_t>(3, 5).size<1>() == 5);
    
    static_assert(layout<int8_t, int32_t , std::pair<int64_t, int64_t>>(5, 3, 1).offset<0>() == 0);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(5, 3, 1).offset<1>() == 8);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(5, 3, 1).offset<2>() == 24);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(5, 3, 1).offsets() == std::array<size_t, 3>{ 0, 8, 24 });
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(3, 5, 7).alloc_size() == 136);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(3, 5, 7).size<0>() == 3);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(3, 5, 7).size<1>() == 5);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(3, 5, 7).size<2>() == 7);

    static_assert(layout<int8_t>::alignment() == 1);
    static_assert(layout<int32_t>::alignment() == 4);
    static_assert(layout<int64_t>::alignment() == 8);
    static_assert(layout<int8_t, int32_t, int64_t>::alignment() == 8);
    static_assert(layout<int8_t, int64_t, int32_t>::alignment() == 8);
    static_assert(layout<int32_t, int8_t, int64_t>::alignment() == 8);
    static_assert(layout<int32_t, int64_t, int8_t>::alignment() == 8);
    static_assert(layout<int64_t, int8_t, int32_t>::alignment() == 8);
    static_assert(layout<int64_t, int32_t, int8_t>::alignment() == 8);

    {
        using L1 = layout<int32_t, int32_t>;
        alignas(max_align_t) unsigned char p[100] = {};
        REQUIRE(reinterpret_cast<unsigned char*>(L1(3, 5).pointer<0>(p)) == p);
        REQUIRE(reinterpret_cast<unsigned char*>(L1(3, 5).pointer<1>(p)) == p + 12);
    }

    {
        using L2 = layout<int8_t, int32_t, std::pair<int64_t, int64_t>>;
        alignas(max_align_t) const unsigned char p[100] = {};
        REQUIRE(reinterpret_cast<const unsigned char*>(L2(5, 3, 1).pointer<0>(p)) == p);
        REQUIRE(reinterpret_cast<const unsigned char*>(L2(5, 3, 1).pointer<1>(p)) == p + 8);
        REQUIRE(reinterpret_cast<const unsigned char*>(L2(5, 3, 1).pointer<2>(p)) == p + 24);
    }

    {
        using L3 = layout<int8_t, int32_t, std::pair<int64_t, int64_t>>;
        alignas(max_align_t) const unsigned char p[100] = {};
        REQUIRE(L3(3, 5, 7).slice<0>(p).size() == 3);
        REQUIRE(L3(3, 5, 7).slice<1>(p).size() == 5);
        REQUIRE(L3(3, 5, 7).slice<2>(p).size() == 7);
    }

}

TEST_CASE("struct")
{
    using T = layout<int, double>;
    constexpr T l(3, 4);

    static_assert(l.type_index<double>() == 1);
    static_assert(l.type_index<int>() == 0);

    static_assert(l.size<0>() == 3);
    static_assert(l.size<1>() == 4);

    static_assert(l.offset<0>() == 0);
    static_assert(l.offset<1>() == 16);

    static_assert(l.alloc_size() == 48);

    struct F
    {
        int a[3];
        double b[4];
        // alignas(int) char* a[sizeof(int) * 3];
        // alignas(double) char* b[sizeof(double) * 3];

        // void show() const
        // {
        //     std::cout << a[0] << a[1] << a[2] << '-'
        //         << b[1] << b[2] << b[3] << b[4] << '\n';
        // }
    };


    static_assert(l.alignment() == 8);
    static_assert(alignof(F) == 8);

    unsigned char* p = new unsigned char[l.alloc_size()];
    F* f = reinterpret_cast<F*>(p);

    f->a[0] = 0;
    f->a[1] = 1;
    f->a[2] = 2;

    f->b[0] = 0;
    f->b[1] = -1;
    f->b[2] = -2;
    f->b[3] = -3;

    // Pointer
    int* ints1 = l.pointer<0>(p);
    double* doubles1 = l.pointer<1>(p);

    REQUIRE(ints1[0] == 0);
    REQUIRE(doubles1[0] == 0);

    // Slice
    auto ints2 = l.slice<0>(p);
    auto doubles2 = l.slice<1>(p);

    REQUIRE(ints2[2] == 2);
    REQUIRE(doubles2[2] == -2);
}

