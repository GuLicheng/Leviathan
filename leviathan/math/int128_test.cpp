#include "int128.hpp"
#include <iostream>
#include <catch2/catch_all.hpp>

using u128 = leviathan::math::uint128_t;

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

using i128 = leviathan::math::int128_t;

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






