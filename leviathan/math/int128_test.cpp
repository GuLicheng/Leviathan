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

TEST_CASE("UnsignedInteger Constructors")
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

TEST_CASE("UnsignedInteger Conversion")
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

TEST_CASE("UnsignedInteger Assignment")
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

    SECTION("AddOperation")
    {
        REQUIRE(UnsignedCheckEqual(Zero + One, One));
        REQUIRE(UnsignedCheckEqual(Zero + Zero, Zero));
        REQUIRE(UnsignedCheckEqual(One + One, Two));
        REQUIRE(UnsignedCheckEqual(One + Two, Three));

        REQUIRE(UnsignedCheckEqual(TwoPower64MinusOne + One, TwoPower64));
        REQUIRE(UnsignedCheckEqual(TwoPower64 + One, TwoPower64PlusOne));
        REQUIRE(UnsignedCheckEqual(Max + One, Zero));   // Warp around
    }

    SECTION("SubOperation")
    {
        REQUIRE(UnsignedCheckEqual(Two - One, One));
        REQUIRE(UnsignedCheckEqual(Three - Two, One));
        REQUIRE(UnsignedCheckEqual(One - Zero, One));
        REQUIRE(UnsignedCheckEqual(Zero - One, Max));   // Warp around

        REQUIRE(UnsignedCheckEqual(TwoPower64 - One, TwoPower64MinusOne));
        REQUIRE(UnsignedCheckEqual(TwoPower64PlusOne - One, TwoPower64));
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

    SECTION("Increase self")
    {
        u128 u = One;

        REQUIRE(++u == Two);
        REQUIRE(u == Two);
        REQUIRE(u++ == Two);
        REQUIRE(u == Three);
    }

    SECTION("Decrease self")
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
    
    SECTION("Popcount")
    {
        REQUIRE(One.popcount() == 1);
        REQUIRE(Zero.popcount() == 0);
        REQUIRE(Max.popcount() == 128);
        REQUIRE(TwoPower64MinusOne.popcount() == 64);
    }

    SECTION("Has single bit")
    {
        REQUIRE(One.has_single_bit() == true);
        REQUIRE(TwoPower64.has_single_bit() == true);
        REQUIRE(Zero.has_single_bit() == false);
    }

    SECTION("Count left zero")
    {
        REQUIRE(One.countl_zero() == 127);
        REQUIRE(Zero.countl_zero() == 128);
        REQUIRE(Max.countl_zero() == 0);
        REQUIRE(TwoPower64.countl_zero() == 63);
    }

    SECTION("Count right zero")
    {
        REQUIRE(One.countr_zero() == 0);
        REQUIRE(Zero.countr_zero() == 128);
        REQUIRE(Max.countr_zero() == 0);
        REQUIRE(TwoPower64.countr_zero() == 64);
    }

    SECTION("Count left one")
    {
        REQUIRE(One.countl_one() == 0);
        REQUIRE(Zero.countl_one() == 0);
        REQUIRE(Max.countl_one() == 128);
    }

    SECTION("Count left zero")
    {
        REQUIRE(One.countl_zero() == 127);
        REQUIRE(Zero.countl_zero() == 128);
        REQUIRE(Max.countl_zero() == 0);
    }
}

// ================================================================================================================

using i128 = leviathan::math::int128_t;

namespace iconstant
{
    
inline constexpr i128 Zero = i128(0, 0);
inline constexpr i128 One = i128(0, 1);
inline constexpr i128 Two = i128(0, 2);
inline constexpr i128 Three = i128(0, 3);
inline constexpr i128 Four = i128(0, 3);
inline constexpr i128 NegativeOne = i128(~int64_t(0), ~int64_t(0));

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
            x = static_cast<T>(-1);
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

TEST_CASE("SignedInteger Constructors")
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












