// https://github.com/abseil/abseil-cpp/blob/master/absl/numeric/int128.h
// https://github.com/dotnet/runtime/blob/5535e31a712343a63f5d7d796cd874e563e5ac14/src/libraries/System.Private.CoreLib/src/System/UInt128.cs
// https://stackoverflow.com/questions/13150449/efficient-unsigned-to-signed-cast-avoiding-implementation-defined-behavior
// https://eisenwave.github.io/cpp-proposals/int-least128.html#example-30d55124
#pragma once

#include <leviathan/extc++/math.hpp>

#include <limits>
#include <compare>
#include <cstdint>
#include <bit>
#include <format>

#include <string>  // TODO
#include <bitset>  // TODO
#include <iostream>

#include <assert.h>

namespace leviathan::math::numeric
{

template <bool Signed, std::endian Endian>
struct int128_layout
{
    using lower_type = uint64_t;
    using upper_type = std::conditional_t<Signed, int64_t, uint64_t>;

    static constexpr bool lower_index = (Endian == std::endian::little);

    uint64_t m_data[2];
    
    constexpr int128_layout() = default;

    constexpr int128_layout(upper_type upper, lower_type lower) : m_data{ static_cast<uint64_t>(upper), lower } { }

    upper_type upper() const { return static_cast<upper_type>(m_data[1 - lower_index]); }

    lower_type lower() const { return static_cast<lower_type>(m_data[lower_index]); }
};

template <std::endian Endian = std::endian::native> class uint128;
template <std::endian Endian = std::endian::native> class int128;

template <std::endian Endian>
class uint128 : int128_layout<false, Endian>
{
    friend class int128<Endian>;

    using base = int128_layout<false, Endian>;

    template <typename T>
    constexpr static uint128 make_uint128_from_float(T v)
    {
        // Undefined behavior if v is NaN or cannot fit into uint128.
        assert(std::isfinite(v) && v >= 0 &&
                (std::numeric_limits<T>::max_exponent <= 128 ||
                v < std::ldexp(static_cast<T>(1), 128)));

        if (v >= std::ldexp(static_cast<T>(1), 64)) 
        {
            const uint64_t hi = static_cast<uint64_t>(std::ldexp(v, -64));
            const uint64_t lo = static_cast<uint64_t>(v - std::ldexp(static_cast<T>(hi), 64));
            return uint128(hi, lo);
        }
        return uint128(0, static_cast<uint64_t>(v));
    }

    template <typename T>
    constexpr static T make_float_from_uint128(uint128 x) 
    { 
        return static_cast<T>(x.lower()) 
             + std::ldexp(static_cast<T>(x.upper()), 64); 
    }

    template <typename Fn>
    constexpr static uint128 bit_op(Fn fn, uint128 u1, uint128 u2)
    {
        const auto hi = fn(u1.upper(), u2.upper());
        const auto lo = fn(u1.lower(), u2.lower());
        return uint128(hi, lo);
    }

public:

    constexpr uint128() = default;
    constexpr uint128(const uint128&) = default;
    constexpr uint128(uint64_t upper, uint64_t lower) : base(upper, lower) { }
    constexpr uint128(int128<Endian> i128) : uint128(static_cast<uint64_t>(i128.upper()), i128.lower()) { }

    // Constructors from unsigned types
    constexpr uint128(uint64_t u) : uint128(0, u) { }
    constexpr uint128(uint32_t u) : uint128(static_cast<uint64_t>(u)) { }
    constexpr uint128(uint16_t u) : uint128(static_cast<uint64_t>(u)) { }
    constexpr uint128(uint8_t u) : uint128(static_cast<uint64_t>(u)) { }

    // Constructors from signed types
    constexpr uint128(int64_t i) : uint128(signbit(i) ? std::numeric_limits<uint64_t>::max() : 0, i) { }
    constexpr uint128(int32_t i) : uint128(static_cast<int64_t>(i)) { }
    constexpr uint128(int16_t i) : uint128(static_cast<int64_t>(i)) { }
    constexpr uint128(int8_t i) : uint128(static_cast<int64_t>(i)) { }

    // Constructors from floating types
    constexpr uint128(float f) : uint128(make_uint128_from_float(f)) { }
    constexpr uint128(double d) : uint128(make_uint128_from_float(d)) { }
    constexpr uint128(long double ld) : uint128(make_uint128_from_float(ld)) { }
    
    // Assignment operator from arithmetic types
    constexpr uint128& operator=(uint128 rhs)
    { 
        this->m_data[0] = rhs.m_data[0];
        this->m_data[1] = rhs.m_data[1];
        return *this;
    }

    constexpr uint128& operator=(uint64_t u) { return *this = uint128(u); }
    constexpr uint128& operator=(uint32_t u) { return *this = uint128(u); }
    constexpr uint128& operator=(uint16_t u) { return *this = uint128(u); }
    constexpr uint128& operator=(uint8_t u) { return *this = uint128(u); }

    constexpr uint128& operator=(int64_t i) { return *this = uint128(i); }
    constexpr uint128& operator=(int32_t i) { return *this = uint128(i); }
    constexpr uint128& operator=(int16_t i) { return *this = uint128(i); }
    constexpr uint128& operator=(int8_t i) { return *this = uint128(i); }

    constexpr uint128& operator=(float f) { return *this = uint128(f); }
    constexpr uint128& operator=(double d) { return *this = uint128(d); }
    constexpr uint128& operator=(long double ld) { return *this = uint128(ld); }

    // Conversion operators to other arithmetic types
    constexpr explicit operator bool(this uint128 x) { return x.lower() || x.upper(); }
    
    constexpr explicit operator char(this uint128 x) { return static_cast<char>(x.lower()); }
    constexpr explicit operator wchar_t(this uint128 x) { return static_cast<wchar_t>(x.lower()); }

    constexpr explicit operator char8_t(this uint128 x) { return static_cast<char8_t>(x.lower()); }
    constexpr explicit operator char16_t(this uint128 x) { return static_cast<char16_t>(x.lower()); }
    constexpr explicit operator char32_t(this uint128 x) { return static_cast<char32_t>(x.lower()); }

    constexpr explicit operator int8_t(this uint128 x) { return static_cast<int8_t>(x.lower()); }
    constexpr explicit operator int16_t(this uint128 x) { return static_cast<int16_t>(x.lower()); }
    constexpr explicit operator int32_t(this uint128 x) { return static_cast<int32_t>(x.lower()); }
    constexpr explicit operator int64_t(this uint128 x) { return static_cast<int64_t>(x.lower()); }

    constexpr explicit operator int128<Endian>(this uint128 x) 
    { 
        return int128<Endian>(
            static_cast<int64_t>(x.upper()),
            x.lower()
        );
    }

    constexpr explicit operator uint8_t(this uint128 x) { return static_cast<uint8_t>(x.lower()); }
    constexpr explicit operator uint16_t(this uint128 x) { return static_cast<uint16_t>(x.lower()); }
    constexpr explicit operator uint32_t(this uint128 x) { return static_cast<uint32_t>(x.lower()); }
    constexpr explicit operator uint64_t(this uint128 x) { return static_cast<uint64_t>(x.lower()); }

    constexpr explicit operator float(this uint128 x) { return make_float_from_uint128<float>(x); }
    constexpr explicit operator double(this uint128 x) { return make_float_from_uint128<double>(x); }
    constexpr explicit operator long double(this uint128 x) { return make_float_from_uint128<long double>(x); }

    // Unary operators
    constexpr uint128 operator+(this uint128 x) { return x; }
    
    constexpr uint128 operator-(this uint128 x)  
    {
        const auto hi = ~x.upper() + static_cast<uint64_t>(x.lower() == 0);
        const auto lo = ~x.lower() + 1;
        return uint128(hi, lo);
    }

    constexpr bool operator!(this uint128 x) { return !static_cast<bool>(x); }

    constexpr uint128 operator~(this uint128 x) { return uint128(~x.upper(), ~x.lower()); }

    // Binary operators
    constexpr uint128 operator|(this uint128 lhs, uint128 rhs) { return bit_op(std::bit_or<>(), lhs, rhs); }
    constexpr uint128 operator&(this uint128 lhs, uint128 rhs) { return bit_op(std::bit_and<>(), lhs, rhs); }
    constexpr uint128 operator^(this uint128 lhs, uint128 rhs) { return bit_op(std::bit_xor<>(), lhs, rhs); }

    constexpr uint128 operator+(this uint128 lhs, uint128 rhs) 
    {
        const auto lo = lhs.lower() + rhs.lower();
        const auto carry = (lo < lhs.lower() ? 1 : 0);
        const auto hi = lhs.upper() + rhs.upper() + carry;
        return uint128(hi, lo);
    }

    constexpr uint128 operator-(this uint128 lhs, uint128 rhs) 
    {
        const auto lo = lhs.lower() - rhs.lower();
        const auto carry = (lo <= lhs.lower() ? 0 : 1);
        const auto hi = lhs.upper() - rhs.upper() - carry;
        return uint128(hi, lo);
    }

    constexpr uint128 operator*(this uint128 lhs, uint128 rhs) 
    {
        // We split uint128 into three parts: 
        // High64bit(H64), Low-High32bit(LH32) and Low-Low32bit(LL32)
        // |----------------|--------|--------|
        // 128             64        32       0
        //        H64           LH32     LL32
        //                           L64
        // A * B = 
        // H64(A) * H64(B) => Overflow
        // H64(A) * L64(B) => H64
        // L64(A) * H64(B) => H64
        // L64(A) * L64(B) =    
        //      LH32(A) * LH32(B) => H64
        //      LL32(A) * LH32(B) => H64 or L64
        //      LH32(A) * LL32(B) => H64 or L64
        //      LL32(A) * LL32(B) => L64

        constexpr uint64_t mask = 0xFFFFFFFF;   // mask low 64-bit

        const uint64_t ah = lhs.lower() >> 32;
        const uint64_t al = lhs.lower() & mask;

        const uint64_t bh = rhs.lower() >> 32;
        const uint64_t bl = rhs.lower() & mask;
        
        const auto part_hi = lhs.upper() * rhs.lower() // H64(A) * L64(B)
                           + lhs.lower() * rhs.upper() // L64(A) * H64(B)
                           + ah * bh;              // LH32(A) * LH32(B)

        const auto part_lo = al * bl; // LL32(A) * LL32(B)

        uint128 result(part_hi, part_lo);

        result += uint128(ah * bl) << 32;  // LH32(A) * LL32(B)
        result += uint128(bh * al) << 32;  // LL32(A) * LH32(B)

        return result;
    }

    constexpr uint128 operator/(this uint128 lhs, uint128 rhs) { return div_mod(lhs, rhs).quotient; }

    constexpr uint128 operator%(this uint128 lhs, uint128 rhs) { return div_mod(lhs, rhs).remainder; }

    constexpr uint128 operator<<(this uint128 lhs, uint64_t amount) 
    {
        // We use uint64_t instead of int to make amount non-negative.
        // The result is undefined if the right operand is negative, or 
        // greater than or equal to the number of bits in the left expression's type.
        assert(amount < 128 && "");

        const auto hi = lhs.upper();
        const auto lo = lhs.lower();
        
        if (amount >= 64)
        {
            return uint128(lo << (amount - 64), 0);
        }
        else if (amount > 0)
        {
            return uint128((hi << amount) | (lo >> (64 - amount)), lo << amount);
        }
        else
        {
            return lhs;
        }
    }

    constexpr uint128 operator>>(this uint128 lhs, uint64_t amount) 
    {
        // We use uint64_t instead of int to make amount non-negative.
        // The result is undefined if the right operand is negative, or 
        // greater than or equal to the number of bits in the left expression's type.
        assert(amount < 128 && "");

        const auto hi = lhs.upper();
        const auto lo = lhs.lower();

        if (amount >= 64)
        {
            return uint128(0, hi >> (amount - 64));
        }
        else if (amount > 0)
        {
            return uint128(hi >> amount, (lo >> amount) | (hi << (64 - amount)));
        }
        else
        {
            return lhs;
        }
    }

    // Comparision
    constexpr bool operator==(this uint128 lhs, uint128 rhs)  
    { 
        return lhs.lower() == rhs.lower()
            && lhs.upper() == rhs.upper();
    }

    constexpr auto operator<=>(this uint128 lhs, uint128 rhs) 
    {
        return lhs.upper() != rhs.upper()
             ? lhs.upper() <=> rhs.upper()
             : lhs.lower() <=> rhs.lower();
    }

    // Other operators
    constexpr uint128& operator+=(uint128 rhs) { return *this = *this + rhs; }
    constexpr uint128& operator-=(uint128 rhs) { return *this = *this - rhs; }
    constexpr uint128& operator*=(uint128 rhs) { return *this = *this * rhs; }
    constexpr uint128& operator/=(uint128 rhs) { return *this = *this / rhs; }
    constexpr uint128& operator%=(uint128 rhs) { return *this = *this % rhs; }
    constexpr uint128& operator|=(uint128 rhs) { return *this = *this | rhs; }
    constexpr uint128& operator^=(uint128 rhs) { return *this = *this ^ rhs; }
    constexpr uint128& operator&=(uint128 rhs) { return *this = *this & rhs; }

    constexpr uint128& operator<<=(int amount) { return *this = *this << amount; }
    constexpr uint128& operator>>=(int amount) { return *this = *this >> amount; }

    constexpr uint128& operator++() { return *this = *this + 1; }

    constexpr uint128& operator--() { return *this = *this - 1; }

    constexpr uint128 operator++(int)
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    constexpr uint128 operator--(int)
    {
        auto temp = *this;
        --*this;
        return temp;
    }

    // Bits
    constexpr friend int popcount(uint128 x) 
    {
        return std::popcount(x.lower()) 
             + std::popcount(x.upper());
    }

    friend constexpr bool has_single_bit(uint128 x) { return popcount(x) == 1; }

    friend constexpr int countl_zero(uint128 x) 
    {
        return x.upper() == 0 
             ? std::countl_zero(x.lower()) + 64
             : std::countl_zero(x.upper());
    } 

    friend constexpr int countr_zero(uint128 x) 
    {
        return x.lower() == 0
             ? std::countr_zero(x.upper()) + 64
             : std::countr_zero(x.lower());
    }

    friend constexpr int countl_one(uint128 x) 
    {
        return x.upper() == std::numeric_limits<uint64_t>::max()
             ? std::countl_one(x.lower()) + 64
             : std::countl_one(x.upper());
    }

    friend constexpr int countr_one(uint128 x) 
    {
        return x.lower() == std::numeric_limits<uint64_t>::max()
             ? std::countr_one(x.upper()) + 64
             : std::countr_one(x.lower());
    }

    std::string to_string(this uint128 x) 
    {
        return std::format("{}", x);
    }

    constexpr uint64_t hash_code(this uint128 x)
    {
        return hash_combine(x.upper(), x.lower());
    }

    static consteval uint128 max() 
    { 
        // 11111...11111
        return uint128(
            std::numeric_limits<uint64_t>::max(), 
            std::numeric_limits<uint64_t>::max()
        );
    }

    static consteval uint128 min() { return uint128(0, 0); }

    static constexpr div_result<uint128> div_mod(uint128 dividend, uint128 divisor)
    {
        assert(divisor != 0 && "dividend = quotient * divisor + remainder");

        // https://stackoverflow.com/questions/5386377/division-without-using
        if (divisor > dividend)
        {
            return { uint128(0), dividend };
        }

        if (divisor == dividend)
        {
            return { uint128(1), uint128(0) };
        }
        
        uint128 denominator = divisor;
        uint128 current = 1;
        uint128 answer = 0;

        // Follow may be faster.
        // const int shift = denominator.countl_zero() - dividend.countl_zero() + 1; 
        const int shift = countl_zero(denominator) - countl_zero(dividend) + 1; 
        denominator <<= shift;
        current <<= shift;

        // After loop, the current will be zero.
        for (int i = 0; i <= shift; ++i)
        {
            if (dividend >= denominator)
            {
                dividend -= denominator;
                answer |= current;
            }
            current >>= 1;
            denominator >>= 1;
        }
        
        return { answer, dividend };
    }

    constexpr auto lower() const { return base::lower(); }

    constexpr auto upper() const { return base::upper(); }

};

template <std::endian Endian>
class int128 : int128_layout<true, Endian>
{
    friend class uint128<Endian>;

    using base = int128_layout<true, Endian>;

    template <typename T>
    static constexpr int128 make_int128_from_float(T v)
    {
        // Conversion when v is NaN or cannot fit into int128 would be undefined
        // behavior if using an intrinsic 128-bit integer.
        assert(std::isfinite(v) && (std::numeric_limits<T>::max_exponent <= 127 ||
                                    (v >= -std::ldexp(static_cast<T>(1), 127) &&
                                     v < std::ldexp(static_cast<T>(1), 127))));

        const uint128<Endian> result = signbit(v) ? -uint128<Endian>(-v) : uint128<Endian>(v);
        return static_cast<int128>(result);
    }

    template <typename T>
    static constexpr T make_float_from_int128(int128 x) 
    {
        // We must convert the absolute value and then negate as needed, because
        // floating point types are typically sign-magnitude. Otherwise, the
        // difference between the high and low 64 bits when interpreted as two's
        // complement overwhelms the precision of the mantissa.
        //
        // Also check to make sure we don't negate Int128Min()
        return x.upper() < 0 && x != int128::min()
            ? -static_cast<T>(-x)
            : static_cast<T>(x.lower()) + std::ldexp(static_cast<T>(x.upper()), 64);
    }

    template <typename Fn>
    static constexpr int128 do_as_uint128(Fn fn, int128 x, int128 y)
    {
        const uint128<Endian> ux = x, uy = y;
        return int128(fn(ux, uy));
    }

public:

    constexpr int128() = default;
    constexpr int128(const int128&) = default;
    constexpr int128(int64_t upper, uint64_t lower) : base(upper, lower) { }
    constexpr int128(uint128<Endian> u128) : int128(static_cast<int64_t>(u128.upper()), u128.lower()) { }

    // COnstructors from unsigned types
    constexpr int128(uint64_t u) : int128(0, u) { }
    constexpr int128(uint32_t u) : int128(static_cast<uint64_t>(u)) { }
    constexpr int128(uint16_t u) : int128(static_cast<uint64_t>(u)) { }
    constexpr int128(uint8_t u) : int128(static_cast<uint64_t>(u)) { }

    // Constructors from signed types
    constexpr int128(int64_t i) : int128((signbit(i) ? ~int64_t(0) : 0), static_cast<uint64_t>(i)) { }
    constexpr int128(int32_t i) : int128(static_cast<int64_t>(i)) { }
    constexpr int128(int16_t i) : int128(static_cast<int64_t>(i)) { }
    constexpr int128(int8_t i) : int128(static_cast<int64_t>(i)) { }

    // Constructors from floating types
    constexpr int128(float f) : int128(make_int128_from_float(f)) { }
    constexpr int128(double d) : int128(make_int128_from_float(d)) { }
    constexpr int128(long double ld) : int128(make_int128_from_float(ld)) { }

    constexpr int128& operator=(int128 rhs)
    {
        this->m_data[0] = rhs.m_data[0];
        this->m_data[1] = rhs.m_data[1];
        return *this;
    }

    constexpr int128& operator=(uint64_t u) { return *this = int128(u); }
    constexpr int128& operator=(uint32_t u) { return *this = int128(u); }
    constexpr int128& operator=(uint16_t u) { return *this = int128(u); }
    constexpr int128& operator=(uint8_t u) { return *this = int128(u); }

    constexpr int128& operator=(int64_t i) { return *this = int128(i); }
    constexpr int128& operator=(int32_t i) { return *this = int128(i); }
    constexpr int128& operator=(int16_t i) { return *this = int128(i); }
    constexpr int128& operator=(int8_t i) { return *this = int128(i); }

    constexpr int128& operator=(float f) { return *this = int128(f); }
    constexpr int128& operator=(double d) { return *this = int128(d); }
    constexpr int128& operator=(long double ld) { return *this = int128(ld); }

    // Conversion operators to other arithmetic types
    constexpr explicit operator bool(this int128 x) { return x.upper() || x.lower(); }
    
    constexpr explicit operator char(this int128 x) { return static_cast<char>(static_cast<int64_t>(x)); }
    constexpr explicit operator wchar_t(this int128 x) { return static_cast<wchar_t>(static_cast<int64_t>(x)); }

    constexpr explicit operator char8_t(this int128 x) { return static_cast<char8_t>(x.lower()); }
    constexpr explicit operator char16_t(this int128 x) { return static_cast<char16_t>(x.lower()); }
    constexpr explicit operator char32_t(this int128 x) { return static_cast<char32_t>(x.lower()); }

    constexpr explicit operator int8_t(this int128 x) { return static_cast<int8_t>(static_cast<int64_t>(x)); }
    constexpr explicit operator int16_t(this int128 x) { return static_cast<int16_t>(static_cast<int64_t>(x)); }
    constexpr explicit operator int32_t(this int128 x) { return static_cast<int32_t>(static_cast<int64_t>(x)); }
    constexpr explicit operator int64_t(this int128 x) { return static_cast<int64_t>(x.lower()); }

    constexpr explicit operator uint8_t(this int128 x) { return static_cast<uint8_t>(x.lower()); }
    constexpr explicit operator uint16_t(this int128 x) { return static_cast<uint16_t>(x.lower()); }
    constexpr explicit operator uint32_t(this int128 x) { return static_cast<uint32_t>(x.lower()); }
    constexpr explicit operator uint64_t(this int128 x) { return static_cast<uint64_t>(x.lower()); }

    constexpr explicit operator uint128<Endian>(this int128 x) 
    {
        return uint128<Endian>(
            static_cast<int64_t>(x.upper()),
            x.lower()
        );
    }

    constexpr explicit operator float(this int128 x) { return make_float_from_int128<float>(x); }
    constexpr explicit operator double(this int128 x) { return make_float_from_int128<double>(x); }
    constexpr explicit operator long double(this int128 x) { return make_float_from_int128<long double>(x); }

    // Unary operators
    constexpr int128 operator+(this int128 x) { return x; }

    constexpr int128 operator-(this int128 x) 
    {
        const uint128<Endian> u = x;
        return -u;
    }

    constexpr bool operator!(this int128 x) { return !static_cast<bool>(x); }

    constexpr int128 operator~(this int128 x) { return int128(~x.upper(), ~x.lower()); }

    // Binary operators
    constexpr int128 operator|(this int128 lhs, int128 rhs) { return do_as_uint128(std::bit_or<>(), lhs, rhs); }
    constexpr int128 operator&(this int128 lhs, int128 rhs) { return do_as_uint128(std::bit_and<>(), lhs, rhs); }
    constexpr int128 operator^(this int128 lhs, int128 rhs) { return do_as_uint128(std::bit_xor<>(), lhs, rhs); }
    constexpr int128 operator+(this int128 lhs, int128 rhs) { return do_as_uint128(std::plus<>(), lhs, rhs); }
    constexpr int128 operator-(this int128 lhs, int128 rhs) { return do_as_uint128(std::minus<>(), lhs, rhs); }
    constexpr int128 operator*(this int128 lhs, int128 rhs) { return do_as_uint128(std::multiplies<>(), lhs, rhs); }
    constexpr int128 operator/(this int128 lhs, int128 rhs) { return div_mod(lhs, rhs).quotient; }
    constexpr int128 operator%(this int128 lhs, int128 rhs) { return div_mod(lhs, rhs).remainder; }

    constexpr int128 operator<<(this int128 lhs, uint64_t amount) 
    {
        // We use uint64_t instead of int to make amount non-negative.
        // The result is undefined if the right operand is negative, or 
        // greater than or equal to the number of bits in the left expression's type.
        assert(amount < 128 && "");
        
        // The shift operation in signed integer and unsigned are same.
        return static_cast<uint128<Endian>>(lhs) << amount;
    }

    // The only difference between signed and unsigned is right shift operation.
    constexpr int128 operator>>(this int128 lhs, uint64_t amount) 
    {
        // We use uint64_t instead of int to make amount non-negative.
        // The result is undefined if the right operand is negative, or 
        // greater than or equal to the number of bits in the left expression's type.
        assert(amount < 128 && "");

        const auto result = static_cast<uint128<Endian>>(lhs) >> amount;
        // Right-shift on signed integral types is an arithmetic right shift, 
        // which performs sign-extension. So we must keep sign bit when shifting 
        // signed integer.
        if (signbit(lhs.upper()))
        {
            return result | (uint128<Endian>::max() << (127 - amount));
        }
        return result;
    }

    constexpr bool operator==(this int128 lhs, int128 rhs) 
    {
        return lhs.upper() == rhs.upper()
            && lhs.lower() == rhs.lower();
    }

    constexpr auto operator<=>(this int128 lhs, int128 rhs) 
    {
        return lhs.upper() == rhs.upper()
             ? lhs.lower() <=> rhs.lower()
             : lhs.upper() <=> rhs.upper();
    }

    // Other operators
    constexpr int128& operator+=(int128 rhs) { return *this = *this + rhs; }
    constexpr int128& operator-=(int128 rhs) { return *this = *this - rhs; }
    constexpr int128& operator*=(int128 rhs) { return *this = *this * rhs; }
    constexpr int128& operator/=(int128 rhs) { return *this = *this / rhs; }
    constexpr int128& operator%=(int128 rhs) { return *this = *this % rhs; }
    constexpr int128& operator|=(int128 rhs) { return *this = *this | rhs; }
    constexpr int128& operator^=(int128 rhs) { return *this = *this ^ rhs; }
    constexpr int128& operator&=(int128 rhs) { return *this = *this & rhs; }

    constexpr int128& operator<<=(int amount) { return *this = *this << amount; }
    constexpr int128& operator>>=(int amount) { return *this = *this >> amount; }

    constexpr int128& operator++() { return *this = *this + 1; }

    constexpr int128& operator--() { return *this = *this - 1; }

    constexpr int128 operator++(int)
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    constexpr int128 operator--(int)
    {
        auto temp = *this;
        --*this;
        return temp;
    }

    constexpr size_t hash_code(this int128 x)
    {
        return hash_combine(x.upper(), x.lower());
    }

    static consteval int128 max() 
    { 
        // 0111111...111
        return int128(
            std::numeric_limits<int64_t>::max(),
            std::numeric_limits<uint64_t>::max()
        );
    }

    static consteval int128 min()
    {
        // 1000000...000
        return int128(
            std::numeric_limits<int64_t>::min(),
            0
        );
    }

    static constexpr div_result<int128> div_mod(int128 dividend, int128 divisor)
    {
        assert(dividend != int128::min() || divisor != -1);

        const uint128<Endian> quotient = dividend < 0 ? -uint128<Endian>(dividend) : uint128<Endian>(dividend);
        const uint128<Endian> remainder = divisor < 0 ? -uint128<Endian>(divisor) : uint128<Endian>(divisor);

        const auto [quot, rem] = uint128<Endian>::div_mod(quotient, remainder);

        return { static_cast<int128>(quot), static_cast<int128>(rem) };
    }

    std::string to_string(this int128 x)
    {
        return std::format("{}", x);
    } 

    constexpr auto lower() const { return base::lower(); }

    constexpr auto upper() const { return base::upper(); }
};

using int128_t = int128<>;
using uint128_t = uint128<>;
// template class uint128<>;
// template class int128<>;

template <std::endian Endian>
std::ostream& operator<<(std::ostream& os, uint128<Endian> x)
{
    return os << x.to_string();
}

template <std::endian Endian>
std::ostream& operator<<(std::ostream& os, int128<Endian> x)
{
    return os << x.to_string();
}

}

// Specialize for std::numeric_limits
template <std::endian Endian>
struct std::numeric_limits<leviathan::math::numeric::uint128<Endian>>
{
private:
    using uint128 = leviathan::math::numeric::uint128<Endian>;

public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr std::float_denorm_style has_denorm = std::float_denorm_style::denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr std::float_round_style round_style = std::float_round_style::round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = true;
    static constexpr int digits = 128;
    static constexpr int digits10 = 38;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;

    static constexpr bool traps = std::numeric_limits<uint64_t>::traps;
    static constexpr bool tinyness_before = false;

    static constexpr uint128 min() { return uint128::min(); }
    static constexpr uint128 lowest() { return 0; }
    static constexpr uint128 max() { return uint128::max(); }
    static constexpr uint128 epsilon() { return 0; }
    static constexpr uint128 round_error() { return 0; }
    static constexpr uint128 infinity() { return 0; }
    static constexpr uint128 quiet_NaN() { return 0; }
    static constexpr uint128 signaling_NaN() { return 0; }
    static constexpr uint128 denorm_min() { return 0; }
};

template <std::endian Endian>
struct std::numeric_limits<leviathan::math::numeric::int128<Endian>>
{
private:
    using int128 = leviathan::math::numeric::int128<Endian>;

public:

    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr std::float_denorm_style has_denorm = std::float_denorm_style::denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr std::float_round_style round_style = std::float_round_style::round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = false;
    static constexpr int digits = 127;
    static constexpr int digits10 = 38;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps = numeric_limits<uint64_t>::traps;
    static constexpr bool tinyness_before = false;

    static constexpr int128 min() { return int128::min(); }
    static constexpr int128 lowest() { return int128::min(); }
    static constexpr int128 max() { return int128::max(); }
    static constexpr int128 epsilon() { return 0; }
    static constexpr int128 round_error() { return 0; }
    static constexpr int128 infinity() { return 0; }
    static constexpr int128 quiet_NaN() { return 0; }
    static constexpr int128 signaling_NaN() { return 0; }
    static constexpr int128 denorm_min() { return 0; }
};

// Specialize for std::hash
template <std::endian Endian>
struct std::hash<leviathan::math::numeric::uint128<Endian>> 
{
    static constexpr auto operator()(leviathan::math::numeric::uint128<Endian> x)
    {
        return x.hash_code();
    }
};

template <std::endian Endian>
struct std::hash<leviathan::math::numeric::int128<Endian>> 
{
    static constexpr auto operator()(leviathan::math::numeric::int128<Endian> x)
    {
        return x.hash_code();
    }
};

// Specialize for std::formatter
template <std::endian Endian, typename CharT>
struct std::formatter<leviathan::math::numeric::uint128<Endian>, CharT>
{
    using uint128 = leviathan::math::numeric::uint128<Endian>;

    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) 
    {
        return m_fmt.parse(ctx);
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(uint128 x, FormatContext& ctx) const
    {
        unsigned __int128 v = (static_cast<unsigned __int128>(x.upper()) << 64) 
                            | (static_cast<unsigned __int128>(x.lower()));
        return m_fmt.format(v, ctx);
    }   

    std::formatter<unsigned __int128, CharT> m_fmt;
};

template <std::endian Endian, typename CharT>
struct std::formatter<leviathan::math::numeric::int128<Endian>, CharT>
{
    using int128 = leviathan::math::numeric::int128<Endian>;

    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) 
    {
        return m_fmt.parse(ctx);
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(int128 x, FormatContext& ctx) const
    {
        signed __int128 v = (static_cast<signed __int128>(x.upper()) << 64) 
                          | (static_cast<signed __int128>(x.lower()));
        return m_fmt.format(v, ctx);
    }   

    std::formatter<signed __int128, CharT> m_fmt;
};

namespace leviathan
{

using leviathan::math::numeric::uint128_t;
using leviathan::math::numeric::int128_t;

}









