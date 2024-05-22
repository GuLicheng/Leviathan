#include "int128.hpp"
#include <iostream>
#include <catch2/catch_all.hpp>

using u128 = leviathan::uint128_t;

namespace uconstant
{

inline constexpr u128 Zero = u128(0, 0);
inline constexpr u128 One = u128(0, 1);
inline constexpr u128 Two = u128(0, 2);
inline constexpr u128 Three = u128(0, 3);
inline constexpr u128 Four = u128(0, 4);

inline constexpr u128 TwoPower64 = u128(1, 0);
inline constexpr u128 TwoPower64PlusOne = u128(1, 1);
inline constexpr u128 TwoPower64MinusOne = u128(0, -1);

inline constexpr u128 DoubleTwoPower64 = u128(2, 0);

inline constexpr u128 Max = u128::max();

}

template <typename... Ts>
struct UnsignedIntegerConstructorTest
{
    static constexpr u128 one = uconstant::One;
    
    void static TestConstructors()
    {
        auto impl = []<typename T>() 
        {
            T temp = 1;
            REQUIRE(one == temp);
        };

        (impl.template operator()<Ts>(), ...);
    }   

    void static TestAssignment()
    {
        auto impl = []<typename T>() 
        {
            u128 x;
            x = static_cast<T>(1);
            REQUIRE(one == x);
        };

        (impl.template operator()<Ts>(), ...);
    }

    void static TestConvert()
    {
        auto impl = []<typename T>() 
        {
            T temp = static_cast<T>(one);
            REQUIRE(one == temp);
        };

        (impl.template operator()<Ts>(), ...);
    }
};

TEST_CASE("UnsignedIntegerConstructors")
{
    UnsignedIntegerConstructorTest<
        int8_t,
        int16_t,
        int32_t,
        int64_t,
        uint8_t,
        uint16_t,
        uint32_t,
        uint64_t,
        float,
        double,
        long double
        >::TestConstructors();
}

TEST_CASE("UnsignedIntegerConversion")
{
    UnsignedIntegerConstructorTest<
        char,
        wchar_t,
        char8_t, 
        char16_t, 
        char32_t,

        int8_t,
        int16_t,
        int32_t,
        int64_t,

        uint8_t,
        uint16_t,
        uint32_t,
        uint64_t,

        float,
        double,
        long double
        >::TestConvert();
}

TEST_CASE("UnsignedIntegerAssignment")
{
    UnsignedIntegerConstructorTest<
        int8_t,
        int16_t,
        int32_t,
        int64_t,
        uint8_t,
        uint16_t,
        uint32_t,
        uint64_t,
        float,
        double,
        long double
        >::TestAssignment();
}

bool UnsignedCheckEqual(u128 x, u128 y)
{
    bool result = x.to_string() == y.to_string();
    if (result)
    {
        REQUIRE(x == y);
    }
    return result;
}

TEST_CASE("UnsignedIntegerUnaryOperation")
{
    using namespace uconstant;

    REQUIRE(UnsignedCheckEqual(-Zero, Zero));
    
    REQUIRE(UnsignedCheckEqual(+Zero, Zero));

    REQUIRE(UnsignedCheckEqual(~Zero, Max));
    REQUIRE(UnsignedCheckEqual(~Max, Zero));

    REQUIRE(!Zero == true);
    REQUIRE(!One == false);
}

TEST_CASE("UnsignedIntegerBinaryOperarion")
{
    using namespace uconstant;

    SECTION("BitOperation")
    {
        REQUIRE(UnsignedCheckEqual(One | Zero, One));
        REQUIRE(UnsignedCheckEqual(Max | Zero, Max));
        
        REQUIRE(UnsignedCheckEqual(Max & Zero, Zero));
        REQUIRE(UnsignedCheckEqual(One & Max, One));

        REQUIRE(UnsignedCheckEqual(One ^ Zero, One));
        REQUIRE(UnsignedCheckEqual(One ^ One, Zero));
    }

    SECTION("EqualOperation")
    {
        // See `UnsignedCheckEqual`
    }

    SECTION("Addition")
    {
        REQUIRE(UnsignedCheckEqual(Zero + One, One));
        REQUIRE(UnsignedCheckEqual(Zero + Zero, Zero));
        REQUIRE(UnsignedCheckEqual(One + One, Two));
        REQUIRE(UnsignedCheckEqual(One + Two, Three));

        REQUIRE(UnsignedCheckEqual(TwoPower64MinusOne + One, TwoPower64));
        REQUIRE(UnsignedCheckEqual(TwoPower64 + One, TwoPower64PlusOne));
        REQUIRE(UnsignedCheckEqual(Max + One, Zero));   // Warp around
    }

    SECTION("Subtraction")
    {
        REQUIRE(UnsignedCheckEqual(Two - One, One));
        REQUIRE(UnsignedCheckEqual(Three - Two, One));
        REQUIRE(UnsignedCheckEqual(One - Zero, One));
        REQUIRE(UnsignedCheckEqual(Zero - One, Max));   // Warp around

        REQUIRE(UnsignedCheckEqual(TwoPower64 - One, TwoPower64MinusOne));
        REQUIRE(UnsignedCheckEqual(TwoPower64PlusOne - One, TwoPower64));
        REQUIRE(UnsignedCheckEqual(Zero - One, Max));
    }

    SECTION("ShiftLeft")
    {
        REQUIRE(UnsignedCheckEqual(One << 1, Two));
        REQUIRE(UnsignedCheckEqual(Two << 1, Four));
        REQUIRE(UnsignedCheckEqual(One << 2, Four));
        REQUIRE(UnsignedCheckEqual(One << 64, TwoPower64));
        REQUIRE(UnsignedCheckEqual(One << 65, DoubleTwoPower64));
    }

    SECTION("ShiftRight")
    {
        REQUIRE(UnsignedCheckEqual(One >> 1, Zero));
        REQUIRE(UnsignedCheckEqual(Two >> 1, One));
        REQUIRE(UnsignedCheckEqual(DoubleTwoPower64 >> 1, TwoPower64));
        REQUIRE(UnsignedCheckEqual(DoubleTwoPower64 >> 100, Zero));
    }

    SECTION("Multiply")
    {
        REQUIRE(UnsignedCheckEqual(Zero * Max, Zero));
        REQUIRE(UnsignedCheckEqual(Zero * One, Zero));
        REQUIRE(UnsignedCheckEqual(Max * One, Max));
        REQUIRE(UnsignedCheckEqual(Two * One, Two));
        REQUIRE(UnsignedCheckEqual(Two * TwoPower64, DoubleTwoPower64));
    }

    SECTION("Comparision")
    {
        REQUIRE(One > Zero);
        REQUIRE(Zero < One);

        REQUIRE(One >= One);
        REQUIRE(One <= One);

        REQUIRE(Max > One);
        REQUIRE(Max > TwoPower64);
        REQUIRE(Max > TwoPower64);
        REQUIRE(Max > TwoPower64MinusOne);

        REQUIRE(One < TwoPower64);        
        REQUIRE(One <= TwoPower64);        
        REQUIRE(TwoPower64MinusOne <= TwoPower64);        
    }

    SECTION("IncreaseSelf")
    {
        u128 u = One;

        REQUIRE(++u == Two);
        REQUIRE(u == Two);
        REQUIRE(u++ == Two);
        REQUIRE(u == Three);
    }

    SECTION("DecreaseSelf")
    {
        u128 u = Two;

        REQUIRE(u-- == Two);
        REQUIRE(u == One);
        REQUIRE(--u == Zero);
        REQUIRE(u == Zero);
    }

    SECTION("Division")
    {
        REQUIRE(One / One == One);
        REQUIRE(One / Two == Zero);
        REQUIRE(Two / One == Two);
        REQUIRE(Three / Two == One);
        REQUIRE(Four / Three == One);
        REQUIRE(Four / Two == Two);
    }

    SECTION("Modulus")
    {
        REQUIRE(One % One == Zero);
        REQUIRE(Two % One == Zero);
        REQUIRE(TwoPower64PlusOne % TwoPower64 == One);
        REQUIRE(TwoPower64MinusOne % TwoPower64 == TwoPower64MinusOne);
    }
}

TEST_CASE("UnsignedIntegerBits")
{
    using namespace uconstant;
    using namespace leviathan::math;

    SECTION("Popcount")
    {
        REQUIRE(popcount(One) == 1);
        REQUIRE(popcount(Zero) == 0);
        REQUIRE(popcount(Max) == 128);
        REQUIRE(popcount(TwoPower64MinusOne) == 64);
    }

    SECTION("Has single bit")
    {
        REQUIRE(has_single_bit(One) == true);
        REQUIRE(has_single_bit(TwoPower64) == true);
        REQUIRE(has_single_bit(Zero) == false);
    }

    SECTION("Count left zero")
    {
        REQUIRE(countl_zero(One) == 127);
        REQUIRE(countl_zero(Zero) == 128);
        REQUIRE(countl_zero(Max) == 0);
        REQUIRE(countl_zero(TwoPower64) == 63);
    }

    SECTION("Count right zero")
    {
        REQUIRE(countr_zero(One) == 0);
        REQUIRE(countr_zero(Zero) == 128);
        REQUIRE(countr_zero(Max) == 0);
        REQUIRE(countr_zero(TwoPower64) == 64);
    }

    SECTION("Count left one")
    {
        REQUIRE(countl_one(One) == 0);
        REQUIRE(countl_one(Zero) == 0);
        REQUIRE(countl_one(Max) == 128);
    }

    SECTION("Count left zero")
    {
        REQUIRE(countl_zero(One) == 127);
        REQUIRE(countl_zero(Zero) == 128);
        REQUIRE(countl_zero(Max) == 0);
    }
}

// ================================================================================================================

using i128 = leviathan::int128_t;

namespace iconstant
{
    
inline constexpr i128 Zero = i128(0, 0);
inline constexpr i128 One = 1;
inline constexpr i128 Two = 2;
inline constexpr i128 Three = 3;
inline constexpr i128 Four = 4;
inline constexpr i128 NegativeOne = -1;
inline constexpr i128 NegativeTwo = -2;

inline constexpr i128 Max = i128::max();
inline constexpr i128 Min = i128::min();

inline constexpr i128 TwoPower64 = i128(1, 0);
inline constexpr i128 TwoPower64PlusOne = i128(1, 1);
inline constexpr i128 TwoPower64MinusOne = i128(0, -1);
inline constexpr i128 DoubleTwoPower64 = i128(2, 0);

} // namespace iconstant

template <typename... Ts>
struct SignedIntegerConstructorTest
{
    static constexpr i128 one = iconstant::One;
    static constexpr i128 negative_one = iconstant::NegativeOne;

    void static TestConstructors()
    {
        auto impl = []<typename T>() 
        {
            T x = 1;
            REQUIRE(one == x);

            if (std::is_signed_v<T>)
            {
                T y = -1;
                REQUIRE(negative_one == y);
            }
        };

        (impl.template operator()<Ts>(), ...);
    }   

    void static TestAssignment()
    {
        auto impl = []<typename T>() 
        {
            i128 x;
            x = static_cast<T>(1);
            REQUIRE(one == x);
        };

        (impl.template operator()<Ts>(), ...);
    }

    void static TestConvert()
    {
        auto impl = []<typename T>() 
        {
            T temp = static_cast<T>(one);
            REQUIRE(one == temp);
        };

        (impl.template operator()<Ts>(), ...);
    }
};

TEST_CASE("SignedIntegerConstructors")
{
    SignedIntegerConstructorTest<
        int8_t,
        int16_t,
        int32_t,
        int64_t,
        uint8_t,
        uint16_t,
        uint32_t,
        uint64_t,
        float,
        double,
        long double
        >::TestConstructors();
}

TEST_CASE("SignedIntegerAssignment")
{
    SignedIntegerConstructorTest<
        int8_t,
        int16_t,
        int32_t,
        int64_t,
        uint8_t,
        uint16_t,
        uint32_t,
        uint64_t,
        float,
        double,
        long double
        >::TestAssignment();
}

TEST_CASE("SignedIntegerConversion")
{
    SignedIntegerConstructorTest<
        char, 
        wchar_t,
        char8_t,
        char16_t,
        char32_t,
        int8_t,
        int16_t,
        int32_t,
        int64_t,
        uint8_t,
        uint16_t,
        uint32_t,
        uint64_t,
        float, 
        double,
        long double
        >::TestConvert();
}

bool SignedCheckEqual(i128 x, i128 y)
{
    bool result = x.to_string() == y.to_string();
    if (result)
    {
        REQUIRE(x == y);
    }
    return result;
}

TEST_CASE("SignedIntegerUnaryOperation")
{
    using namespace iconstant;

    REQUIRE(SignedCheckEqual(-Zero, Zero));
    
    REQUIRE(SignedCheckEqual(+Zero, Zero));

    REQUIRE(SignedCheckEqual(~Zero, NegativeOne));
    REQUIRE(SignedCheckEqual(~NegativeOne, Zero));

    REQUIRE(!Zero == true);
    REQUIRE(!One == false);
}

TEST_CASE("SignedIntegerBinaryOperation")
{
    using namespace iconstant;

    SECTION("EqualOperation")
    {
        // See `SignedCheckEqual`
    }

    SECTION("Addition")
    {
        REQUIRE(SignedCheckEqual(One + One, Two));
        REQUIRE(SignedCheckEqual(Zero + One, One));
        REQUIRE(SignedCheckEqual(NegativeOne + One, Zero));
        REQUIRE(SignedCheckEqual(TwoPower64MinusOne + Two, TwoPower64PlusOne));
    }

    SECTION("Subtraction")
    {
        REQUIRE(SignedCheckEqual(One - One, Zero));
        REQUIRE(SignedCheckEqual(Zero - One, NegativeOne));
        REQUIRE(SignedCheckEqual(One - Two, NegativeOne));
        REQUIRE(SignedCheckEqual(TwoPower64PlusOne - Two, TwoPower64MinusOne));
    }

    SECTION("ShiftLeft")
    {
        REQUIRE(SignedCheckEqual(One << 1, 2));
        REQUIRE(SignedCheckEqual(Two << 1, 4));
        REQUIRE(SignedCheckEqual(NegativeOne << 1, -2));
        REQUIRE(SignedCheckEqual(Zero << 1, Zero));
        REQUIRE(SignedCheckEqual(TwoPower64 << 1, DoubleTwoPower64));
    }

    SECTION("ShiftRight")
    {
        REQUIRE(SignedCheckEqual(One >> 1, Zero));
        REQUIRE(SignedCheckEqual(Zero >> 1, Zero));
        REQUIRE(SignedCheckEqual(DoubleTwoPower64 >> 1, TwoPower64));
        REQUIRE((NegativeOne >> 1) == NegativeOne);
        REQUIRE(SignedCheckEqual(i128(-2) >> 1, NegativeOne));
    }

    SECTION("Multiply")
    {
        REQUIRE(UnsignedCheckEqual(Zero * Max, Zero));
        REQUIRE(UnsignedCheckEqual(Zero * Min, Zero));
        REQUIRE(UnsignedCheckEqual(Zero * Zero, Zero));
        REQUIRE(UnsignedCheckEqual(One * One, One));
        REQUIRE(UnsignedCheckEqual(NegativeOne * One, NegativeOne));
    }

}

// Follow testing is copied from https://github.com/chfast/intx/blob/master/test/unittests/test_int128.cpp
// ================================================================================================================
#include <string>

template <typename T>
[[noreturn]] inline void throw_(const char* what)
{
#if __cpp_exceptions
    throw T{what};
#else
    std::fputs(what, stderr);
    std::abort();
#endif
}

inline constexpr int from_dec_digit(char c)
{
    if (c < '0' || c > '9')
        throw_<std::invalid_argument>("invalid digit");
    return c - '0';
}

inline constexpr int from_hex_digit(char c)
{
    if (c >= 'a' && c <= 'f')
        return c - ('a' - 10);
    if (c >= 'A' && c <= 'F')
        return c - ('A' - 10);
    return from_dec_digit(c);
}

template <typename Int>
inline constexpr Int from_string(const char* str)
{
    auto s = str;
    auto x = Int{};
    int num_digits = 0;

    if (s[0] == '0' && s[1] == 'x')
    {
        s += 2;
        while (const auto c = *s++)
        {
            if (++num_digits > int{sizeof(x) * 2})
                throw_<std::out_of_range>(str);
            x = (x << uint64_t{4}) | from_hex_digit(c);
        }
        return x;
    }

    while (const auto c = *s++)
    {
        if (num_digits++ > std::numeric_limits<Int>::digits10)
            throw_<std::out_of_range>(str);

        const auto d = from_dec_digit(c);
        x = x * Int{10} + d;
        if (x < d)
            throw_<std::out_of_range>(str);
    }

    return x;
}

template <typename Int>
inline constexpr Int from_string(const std::string& s)
{
    return from_string<Int>(s.c_str());
}

struct Uint128Wrapper
{
    u128 value;

    constexpr Uint128Wrapper(uint64_t lo, uint64_t hi) : value(hi, lo) { }

    constexpr Uint128Wrapper(u128 u) : value(u) { }

    template <std::integral Int>
    constexpr Uint128Wrapper(Int i) : value(i) { }
};

constexpr Uint128Wrapper operator""_u128(const char* s)
{
    return from_string<u128>(s);
}

namespace intx
{

struct ArithTestCase
{
    Uint128Wrapper x;
    Uint128Wrapper y;
    Uint128Wrapper sum;
    Uint128Wrapper difference;
    Uint128Wrapper product;
};

static ArithTestCase arith_test_cases[] = {
    {0, 0, 0, 0, 0},
    {0, 1, 1, 0xffffffffffffffffffffffffffffffff_u128, 0},
    {1, 0, 1, 1, 0},
    {1, 1, 2, 0, 1},
    {1, 0xffffffffffffffff, {0, 1}, 0xffffffffffffffff0000000000000002_u128, 0xffffffffffffffff},
    {0xffffffffffffffff, 1, {0, 1}, 0xfffffffffffffffe, 0xffffffffffffffff},
    {0xffffffffffffffff, 0xffffffffffffffff, 0x1fffffffffffffffe_u128, 0,
     0xfffffffffffffffe0000000000000001_u128},
    {0x8000000000000000, 0x8000000000000000, {0, 1}, 0, 0x40000000000000000000000000000000_u128},
    {0x18000000000000000_u128, 0x8000000000000000, 0x20000000000000000_u128,
     0x10000000000000000_u128, 0xc0000000000000000000000000000000_u128},
    {0x8000000000000000, 0x18000000000000000_u128, 0x20000000000000000_u128,
     0xffffffffffffffff0000000000000000_u128, 0xc0000000000000000000000000000000_u128},
    {{0, 1}, 0xffffffffffffffff, 0x1ffffffffffffffff_u128, 1, 0xffffffffffffffff0000000000000000_u128},
    {{0, 1}, {0, 1}, 0x20000000000000000_u128, 0, 0},
};
}


void EXPECT_EQ(auto a, auto b)
{
    auto x = std::format("{}", a);
    auto y = std::format("{}", b);
    CHECK(x == y);
}

TEST_CASE("intx add")
{
    for (const auto& t : intx::arith_test_cases)
    {
        EXPECT_EQ(t.x.value + t.y.value, t.sum.value);
        EXPECT_EQ(t.y.value + t.x.value, t.sum.value);
    }
}

TEST_CASE("intx sub")
{
    for (const auto& t : intx::arith_test_cases)
    {
        EXPECT_EQ(t.x.value - t.y.value, t.difference.value);
    }
}

TEST_CASE("intx mul")
{
    for (const auto& t : intx::arith_test_cases)
    {
        EXPECT_EQ(t.x.value * t.y.value, t.product.value);
        EXPECT_EQ(t.y.value * t.x.value, t.product.value);
        auto z = t.x.value;
        EXPECT_EQ(z *= t.y.value, t.product.value);
        EXPECT_EQ(z, t.product.value);
    }
}

void Check(auto a, auto b)
{
    auto aa = std::format("{}", a); 
    auto bb = std::format("{}", b); 
    CHECK(aa == bb);
}

unsigned __int128 MakeBuiltinUnsigned(size_t hi, size_t lo)
{
    return static_cast<unsigned __int128>(hi) << 64 | static_cast<unsigned __int128>(lo);
}

void RandomTestForDivAndMod()
{
    static std::random_device rd;

    const auto hi1 = rd();
    const auto lo1 = rd();

    const auto a1 = MakeBuiltinUnsigned(hi1, lo1);
    const auto b1 = u128(hi1, lo1);

    const auto hi2 = rd();
    const auto lo2 = rd();

    const auto a2 = MakeBuiltinUnsigned(hi2, lo2);
    const auto b2 = u128(hi2, lo2);

    Check(a1, b1);
    Check(a2, b2);

    if (a1 == 0)
    {
        return; // Avoid divide-by-zero.
    }

    Check(a2 / a1, b2 / b1);
    Check(a2 % a1, b2 % b1);

    const auto [q, r] = u128::div_mod(b1, b2);
    auto c = b2 * q + r;
    CHECK(b1 == c);
}

TEST_CASE("DivideAndModRandomInputs")
{
    const int kNumIters = 1 << 18;
    for (int i = 0; i < kNumIters; ++i) 
    {
        RandomTestForDivAndMod();
    }
}

TEST_CASE("DivideAndMod")
{
    using std::swap;

    // a := q * b + r
    u128 a, b, q, r;

    // Zero test.
    a = 0;
    b = 123;
    q = a / b;
    r = a % b;
    EXPECT_EQ(0, q);
    EXPECT_EQ(0, r);

    a = u128(0x530eda741c71d4c3, 0xbf25975319080000);
    q = u128(0x4de2cab081, 0x14c34ab4676e4bab);
    b = u128(0x1110001);
    r = u128(0x3eb455);
    EXPECT_EQ(a, q * b + r); // Sanity-check.

    u128 result_q, result_r;
    result_q = a / b;
    result_r = a % b;
    EXPECT_EQ(q, result_q);
    EXPECT_EQ(r, result_r);

    // Try the other way around.
    swap(q, b);
    result_q = a / b;
    result_r = a % b;
    EXPECT_EQ(q, result_q);
    EXPECT_EQ(r, result_r);
    // Restore.
    swap(b, q);

    // Dividend < divisor; result should be q:0 r:<dividend>.
    swap(a, b);
    result_q = a / b;
    result_r = a % b;
    EXPECT_EQ(0, result_q);
    EXPECT_EQ(a, result_r);
    // Try the other way around.
    swap(a, q);
    result_q = a / b;
    result_r = a % b;
    EXPECT_EQ(0, result_q);
    EXPECT_EQ(a, result_r);
    // Restore.
    swap(q, a);
    swap(b, a);

    // Try a large remainder.
    b = a / 2 + 1;
    u128 expected_r = u128(0x29876d3a0e38ea61, 0xdf92cba98c83ffff);
    // Sanity checks.
    EXPECT_EQ(a / 2 - 1, expected_r);
    EXPECT_EQ(a, b + expected_r);
    result_q = a / b;
    result_r = a % b;
    EXPECT_EQ(1, result_q);
    EXPECT_EQ(expected_r, result_r);
}

// As we all known, some complier provide builtin type __int128, we
// can use it to help us test.
namespace builtin_type_test
{

enum BinOp : int
{
    Add = 0,
    Sub = 1,
    Mul = 2,
    Div = 3,
    Mod = 4,    

    Size,
};

std::pair<unsigned __int128, u128> RandUint128()
{
    static std::random_device rd;
    const auto hi = rd(), lo = rd();

    const auto a = MakeBuiltinUnsigned(hi, lo);
    const auto b = u128(hi, lo);
    
    return { a, b };
}

void BinInvokeTest(auto fn, auto a1, auto a2, auto b1, auto b2)
{
    const auto a = fn(a1, a2);
    const auto b = fn(b1, b2);
    EXPECT_EQ(a, b);
}

void UnInvokeTest(auto fn, auto a1, auto b1)
{
    EXPECT_EQ(fn(a1), fn(b1));
}

void RandomOperationTest()
{
    auto [a1, b1] = RandUint128();
    auto [a2, b2] = RandUint128();

    constexpr auto Inverse = [](auto x) static 
    {
        return ~x;
    };

    constexpr auto Increment = [](auto x) static
    {
        ++x;
        x++;
        return x;
    };  

    constexpr auto Decrement = [](auto x) static
    {
        --x;
        x--;
        return x;
    };  

    // TODO
    constexpr auto ShiftLeft = [](auto x, auto amount) static
    {
        return x << ((amount) % 128);
    };

    constexpr auto ShiftRight = [](auto x, auto amount) static
    {
        return x >> ((amount) % 128);
    };

    UnInvokeTest(std::negate<>(), a1, b1);
    UnInvokeTest(std::bit_not<>(), a1, b1);
    UnInvokeTest(Inverse, a1, b1);
    UnInvokeTest(Increment, a1, b1);
    UnInvokeTest(Decrement, a1, b1);

    static std::random_device rd;
    const auto amount = rd();
    BinInvokeTest(ShiftLeft, a1, amount, b1, amount);
    BinInvokeTest(ShiftRight, a1, amount, b1, amount);


    BinInvokeTest(std::plus<>(), a1, a2, b1, b2);
    BinInvokeTest(std::minus<>(), a1, a2, b1, b2);
    BinInvokeTest(std::multiplies<>(), a1, a2, b1, b2);
    BinInvokeTest(std::bit_and<>(), a1, a2, b1, b2);
    BinInvokeTest(std::bit_or<>(), a1, a2, b1, b2);
    BinInvokeTest(std::bit_xor<>(), a1, a2, b1, b2);

    BinInvokeTest(std::equal_to<>(), a1, a2, b1, b2);
    BinInvokeTest(std::not_equal_to<>(), a1, a2, b1, b2);
    BinInvokeTest(std::less<>(), a1, a2, b1, b2);
    BinInvokeTest(std::less_equal<>(), a1, a2, b1, b2);
    BinInvokeTest(std::greater<>(), a1, a2, b1, b2);
    BinInvokeTest(std::greater_equal<>(), a1, a2, b1, b2);

    
    if (a2 != 0)
    {
        BinInvokeTest(std::minus<>(), a1, a2, b1, b2);
        BinInvokeTest(std::modulus<>(), a1, a2, b1, b2);
    }
}

} // namespace builtin_test

TEST_CASE("RandomTestWithBuiltinType")
{
    for (int i = 0; i < 10000; ++i)
    {
        builtin_type_test::RandomOperationTest();
    }
}



