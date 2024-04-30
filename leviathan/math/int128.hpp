// https://github.com/abseil/abseil-cpp/blob/master/absl/numeric/int128.h
// https://github.com/dotnet/runtime/blob/5535e31a712343a63f5d7d796cd874e563e5ac14/src/libraries/System.Private.CoreLib/src/System/UInt128.cs
// https://stackoverflow.com/questions/13150449/efficient-unsigned-to-signed-cast-avoiding-implementation-defined-behavior
#pragma once

#include "core.hpp"

#include <limits>
#include <compare>
#include <cstdint>
#include <bit>

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

template <std::endian Endian = std::endian::native> struct uint128;
template <std::endian Endian = std::endian::native> struct int128;

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

    constexpr auto lower() const { return base::lower(); }

    constexpr auto upper() const { return base::upper(); }

    template <typename T>
    constexpr T make_float_from_uint128() const
    { 
        return static_cast<T>(lower()) 
             + std::ldexp(static_cast<T>(upper()), 64); 
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
    constexpr uint128(int128<Endian> i128) : base(static_cast<uint64_t>(i128.upper()), i128.lower()) { }

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
    constexpr explicit operator bool() const { return lower() || upper(); }
    
    constexpr explicit operator char() const { return static_cast<char>(lower()); }
    constexpr explicit operator wchar_t() const { return static_cast<wchar_t>(lower()); }

    constexpr explicit operator char8_t() const { return static_cast<char8_t>(lower()); }
    constexpr explicit operator char16_t() const { return static_cast<char16_t>(lower()); }
    constexpr explicit operator char32_t() const { return static_cast<char32_t>(lower()); }

    constexpr explicit operator int8_t() const { return static_cast<int8_t>(lower()); }
    constexpr explicit operator int16_t() const { return static_cast<int16_t>(lower()); }
    constexpr explicit operator int32_t() const { return static_cast<int32_t>(lower()); }
    constexpr explicit operator int64_t() const { return static_cast<int64_t>(lower()); }

    constexpr explicit operator int128<Endian>() const 
    { 
        return int128<Endian>(
            static_cast<int64_t>(upper()),
            lower()
        );
    }

    constexpr explicit operator uint8_t() const { return static_cast<uint8_t>(lower()); }
    constexpr explicit operator uint16_t() const { return static_cast<uint16_t>(lower()); }
    constexpr explicit operator uint32_t() const { return static_cast<uint32_t>(lower()); }
    constexpr explicit operator uint64_t() const { return static_cast<uint64_t>(lower()); }

    constexpr explicit operator float() const { return make_float_from_uint128<float>(); }
    constexpr explicit operator double() const { return make_float_from_uint128<double>(); }
    constexpr explicit operator long double() const { return make_float_from_uint128<long double>(); }

    // Unary operators
    constexpr uint128 operator+() const { return *this; }
    
    constexpr uint128 operator-() const 
    {
        const auto hi = ~upper() + static_cast<uint64_t>(lower() == 0);
        const auto lo = ~lower() + 1;
        return uint128(hi, lo);
    }

    constexpr bool operator!() const { return !static_cast<bool>(*this); }

    constexpr uint128 operator~() const { return uint128(~upper(), ~lower()); }

    // Binary operators
    constexpr uint128 operator|(uint128 rhs) const { return bit_op(std::bit_or<>(), *this, rhs); }
    constexpr uint128 operator&(uint128 rhs) const { return bit_op(std::bit_and<>(), *this, rhs); }
    constexpr uint128 operator^(uint128 rhs) const { return bit_op(std::bit_xor<>(), *this, rhs); }

    constexpr uint128 operator+(uint128 rhs) const
    {
        const auto lo = lower() + rhs.lower();
        const auto carry = (lo < lower() ? 1 : 0);
        const auto hi = upper() + rhs.upper() + carry;
        return uint128(hi, lo);
    }

    constexpr uint128 operator-(uint128 rhs) const
    {
        const auto lo = lower() - rhs.lower();
        const auto carry = (lo <= lower() ? 0 : 1);
        const auto hi = upper() - rhs.upper() - carry;
        return uint128(hi, lo);
    }

    constexpr uint128 operator*(uint128 rhs) const
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

        constexpr uint64_t mask = 0xffffffff;   // mask low 64-bit

        const uint64_t ah = lower() >> 32;
        const uint64_t al = lower() & mask;

        const uint64_t bh = rhs.lower() >> 32;
        const uint64_t bl = rhs.lower() & mask;
        
        const auto part_hi = upper() * rhs.lower() // H64(A) * L64(B)
                           + lower() * rhs.upper() // L64(A) * H64(B)
                           + ah * bh;                              // LH32(A) * LH32(B)

        const auto part_lo = al * bl; // LL32(A) * LL32(B)

        uint128 result(part_hi, part_lo);

        result += uint128(ah * bl) << 32;  // LH32(A) * LL32(B)
        result += uint128(bh * al) << 32;  // LL32(A) * LH32(B)

        return result;
    }

    constexpr uint128 operator/(uint128 rhs) const  { return div_mod(*this, rhs).quotient; }

    constexpr uint128 operator%(uint128 rhs) const  { return div_mod(*this, rhs).remainder; }

    constexpr uint128 operator<<(int amount) const
    {
        assert(0 < amount && amount < 128 && "");

        const auto hi = upper();
        const auto lo = lower();

        return amount >= 64 
             ? uint128(lo << (amount - 64), 0)
             : uint128((hi << amount) | (lo >> (64 - amount)), lo << amount);
    }

    constexpr uint128 operator>>(int amount) const
    {
        assert(0 < amount && amount < 128 && "");

        const auto hi = upper();
        const auto lo = lower();

        return amount >= 64 
             ? uint128(0, hi >> (amount - 64)) 
             : uint128(hi >> amount, (lo >> amount) | (hi << (64 - amount)));
    }

    // Comparision
    constexpr bool operator==(uint128 rhs) const 
    { 
        return lower() == rhs.lower()
            && upper() == rhs.upper();
    }

    constexpr auto operator<=>(uint128 rhs) const
    {
        return upper() != rhs.upper()
            ? upper() <=> rhs.upper()
            : lower() <=> rhs.lower();
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

    std::string to_string(int base = 10) const
    {
        assert((base == 10  || base == 2 ) && "Only supported dec now.");

        if (base == 2)
        {
            std::bitset<64> b1 = upper(), b2 = lower();
            return b1.to_string() + b2.to_string();
        }

        static char base10[] = 
        {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        };

        auto current = *this;
        
        if (current == 0)
        {
            return "0";
        }

        std::string result;

        while (current)
        {
            auto [quotient, remainder] = div_mod(current, 10);
            result += base10[static_cast<int>(remainder)];
            current = quotient;
        }

        return { result.rbegin(), result.rend() };
    }

    constexpr uint64_t hash_code() const { return hash_combine(upper(), lower()); }

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

};

template <std::endian Endian>
class int128 : int128_layout<true, Endian>
{
    friend class uint128<Endian>;

    using base = int128_layout<true, Endian>;

    template <typename T>
    constexpr int128 make_int128_from_float(T v)
    {
        // Conversion when v is NaN or cannot fit into int128 would be undefined
        // behavior if using an intrinsic 128-bit integer.
        assert(std::isfinite(v) && (std::numeric_limits<T>::max_exponent <= 127 ||
                                    (v >= -std::ldexp(static_cast<T>(1), 127) &&
                                     v < std::ldexp(static_cast<T>(1), 127))));

        const uint128<Endian> result = signbit(v) ? -uint128<Endian>(-v) : uint128<Endian>(v);
        return static_cast<int128>(result);
    }

    constexpr auto lower() const { return base::lower(); }

    constexpr auto upper() const { return base::upper(); }

    template <typename T>
    constexpr T make_float_from_int128() const
    {
        // We must convert the absolute value and then negate as needed, because
        // floating point types are typically sign-magnitude. Otherwise, the
        // difference between the high and low 64 bits when interpreted as two's
        // complement overwhelms the precision of the mantissa.
        //
        // Also check to make sure we don't negate Int128Min()
        return upper() < 0 && *this != int128::min()
            ? -static_cast<T>(-*this)
            : static_cast<T>(lower()) + std::ldexp(static_cast<T>(upper()), 64);
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
    constexpr int128(uint128<Endian> u128) : base(static_cast<int64_t>(u128.upper()), u128.lower()) { }

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
    constexpr explicit operator bool() const { return upper() || lower(); }
    
    constexpr explicit operator char() const { return static_cast<char>(static_cast<int64_t>(*this)); }
    constexpr explicit operator wchar_t() const { return static_cast<wchar_t>(static_cast<int64_t>(*this)); }

    constexpr explicit operator char8_t() const { return static_cast<char8_t>(lower()); }
    constexpr explicit operator char16_t() const { return static_cast<char16_t>(lower()); }
    constexpr explicit operator char32_t() const { return static_cast<char32_t>(lower()); }

    constexpr explicit operator int8_t() const { return static_cast<int8_t>(static_cast<int64_t>(*this)); }
    constexpr explicit operator int16_t() const { return static_cast<int16_t>(static_cast<int64_t>(*this)); }
    constexpr explicit operator int32_t() const { return static_cast<int32_t>(static_cast<int64_t>(*this)); }
    constexpr explicit operator int64_t() const { return static_cast<int64_t>(lower()); }

    constexpr explicit operator uint8_t() const { return static_cast<uint8_t>(lower()); }
    constexpr explicit operator uint16_t() const { return static_cast<uint16_t>(lower()); }
    constexpr explicit operator uint32_t() const { return static_cast<uint32_t>(lower()); }
    constexpr explicit operator uint64_t() const { return static_cast<uint64_t>(lower()); }

    constexpr explicit operator uint128<Endian>() const 
    {
        return uint128<Endian>(
            static_cast<int64_t>(upper()),
            lower()
        );
    }

    constexpr explicit operator float() const { return make_float_from_int128<float>(); }
    constexpr explicit operator double() const { return make_float_from_int128<double>(); }
    constexpr explicit operator long double() const { return make_float_from_int128<long double>(); }

    // Unary operators
    constexpr int128 operator+() const { return *this; }

    constexpr int128 operator-() const 
    {
        const uint128<Endian> u = *this;
        return -u;
    }

    constexpr bool operator!() const { return !static_cast<bool>(*this); }

    constexpr int128 operator~() const { return int128(~upper(), ~lower()); }

    // Binary operators
    constexpr int128 operator|(int128 rhs) const { return do_as_uint128(std::bit_or<>(), *this, rhs); }
    constexpr int128 operator&(int128 rhs) const { return do_as_uint128(std::bit_and<>(), *this, rhs); }
    constexpr int128 operator^(int128 rhs) const { return do_as_uint128(std::bit_xor<>(), *this, rhs); }

    constexpr int128 operator+(int128 rhs) const { return do_as_uint128(std::plus<>(), *this, rhs); }

    constexpr int128 operator-(int128 rhs) const { return do_as_uint128(std::minus<>(), *this, rhs); }

    constexpr int128 operator*(int128 rhs) const { return do_as_uint128(std::multiplies<>(), *this, rhs); }
    
    constexpr int128 operator/(int128 rhs) const { return div_mod(*this, rhs).quotient; }

    constexpr int128 operator%(int128 rhs) const { return div_mod(*this, rhs).remainder; }

    constexpr int128 operator<<(int amount) const
    {
        assert(0 < amount && amount < 127 && "int64_t shifts of >= 63 are undefined.");

        // The shift operation in signed integer and unsigned are same.
        return static_cast<uint128<Endian>>(*this) << amount;
    }

    constexpr int128 operator>>(int amount) const
    {
        assert(0 < amount && amount < 127 && "int64_t shifts of >= 63 are undefined.");

        const auto result = static_cast<uint128<Endian>>(*this) >> amount;
        const auto is_negative = signbit(upper());
        // Right-shift on signed integral types is an arithmetic right shift, 
        // which performs sign-extension. So wee must keep sign bit when shifting 
        // signed integer.
        if (is_negative)
        {
            return result | (uint128<Endian>::max() << (127 - amount));
        }
        return result;
    }

    constexpr bool operator==(int128 rhs) const
    {
        return upper() == rhs.upper()
            && lower() == rhs.lower();
    }

    constexpr auto operator<=>(int128 rhs) const
    {
        return upper() == rhs.upper()
             ? lower() <=> rhs.lower()
             : upper() <=> rhs.upper();
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

    std::string to_string(int base = 10) const
    {
        uint128<Endian> u(*this);
        if (signbit(upper()))
        {
            return std::string("-") + (-u).to_string(base);
        }
        return u.to_string(base);
    }

    constexpr size_t hash_code() const { return hash_combine(upper(), lower()); }

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

    friend std::ostream& operator<<(std::ostream& os, int128 x) 
    {
        return os << x.to_string(2);
    }
};

using int128_t = int128<>;
using uint128_t = uint128<>;
template class uint128<>;
template class int128<>;

}

namespace leviathan::math
{

using leviathan::math::numeric::uint128_t;
using leviathan::math::numeric::int128_t;

}

// Extend std::numeric_limits
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

// Extend std::hash
template <std::endian Endian>
struct std::hash<leviathan::math::numeric::uint128<Endian>> 
{
    static constexpr operator()(leviathan::math::numeric::uint128<Endian> x)
    {
        return x.hash_code();
    }
};

template <std::endian Endian>
struct std::hash<leviathan::math::numeric::int128<Endian>> 
{
    static constexpr operator()(leviathan::math::numeric::int128<Endian> x)
    {
        return x.hash_code();
    }
};




