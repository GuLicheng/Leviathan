// https://github.com/abseil/abseil-cpp/blob/master/absl/numeric/int128.h
// https://github.com/dotnet/runtime/blob/5535e31a712343a63f5d7d796cd874e563e5ac14/src/libraries/System.Private.CoreLib/src/System/UInt128.cs
#pragma once

#include "core.hpp"

#include <limits>
#include <compare>
#include <cstdint>
#include <bit>

#include <string>  // TODO
#include <bitset>  // TODO

#include <assert.h>

namespace leviathan::math
{

template <bool Signed, std::endian Endian = std::endian::native>
struct endian
{
    using lower_type = uint64_t;
    using upper_type = std::conditional_t<Signed, int64_t, uint64_t>;

    static constexpr bool lower_index = (Endian == std::endian::little);

    uint64_t m_data[2];
    
    constexpr endian() = default;

    constexpr endian(upper_type upper, lower_type lower) : m_data{ upper, lower } { }

    upper_type upper() const { return static_cast<upper_type>(m_data[1 - lower_index]); }

    lower_type lower() const { return static_cast<lower_type>(m_data[lower_index]); }
};

using uint128_endian = endian<false>;
using int128_endian = endian<true>;

template <typename Endian = uint128_endian> struct uint128;
template <typename Endian = int128_endian> struct int128;

template <typename Endian>
class uint128
{
    // template <typename E>
    friend class int128<Endian>;

    template <typename T>
    static uint128 make_uint128_from_float(T v)
    {
        // Undefined behavior if v is NaN or cannot fit into uint128.
        assert(std::isfinite(v) && v >= 0 &&
                (std::numeric_limits<T>::max_exponent <= 128 ||
                v < std::ldexp(static_cast<T>(1), 128)));

        if (v >= std::ldexp(static_cast<T>(1), 64)) 
        {
            uint64_t hi = static_cast<uint64_t>(std::ldexp(v, -64));
            uint64_t lo = static_cast<uint64_t>(v - std::ldexp(static_cast<T>(hi), 64));
            return uint128(hi, lo);
        }
        return uint128(0, static_cast<uint64_t>(v));
    }

    template <typename T>
    T make_float_from_uint128() const
    { 
        return static_cast<T>(m_value.lower()) 
             + std::ldexp(static_cast<T>(m_value.upper()), 64); 
    }

    template <typename Fn>
    static uint128 bit_op(Fn fn, uint128 u1, uint128 u2)
    {
        const auto hi = fn(u1.m_value.upper(), u2.m_value.upper());
        const auto lo = fn(u1.m_value.lower(), u2.m_value.lower());
        return uint128(hi, lo);
    }

    static div_result<uint128> div_mod(uint128 dividend, uint128 divisor)
    {
        assert(divisor != 0 && "dividend = quotient * divisor + remainder");

        // https://stackoverflow.com/questions/5386377/division-without-using
        if (divisor > dividend)
        {
            return { .quotient = uint128(0), .remainder = dividend };
        }

        if (divisor == dividend)
        {
            return { .quotient = uint128(1), .remainder = uint128(0) };
        }
        
        uint128 denominator = divisor;
        uint128 current = 1;
        uint128 answer = 0;

        // while (denominator <= dividend)
        // {
        //     denominator <<= 1;
        //     current <<= 1;
        // }

        // denominator >>= 1;
        // current >>= 1;

        // Follow may be faster.
        const int shift = denominator.countl_zero() - dividend.countl_zero() + 1; 
        denominator <<= shift;
        current <<= shift;

        while (current)
        {
            if (dividend >= denominator)
            {
                dividend -= denominator;
                answer |= current;
            }
            current >>= 1;
            denominator >>= 1;
        }
        
        return { .quotient = answer, .remainder = dividend };
    }

public:

    constexpr uint128() = default;
    constexpr uint128(const uint128&) = default;
    constexpr uint128(typename Endian::upper_type upper, typename Endian::lower_type lower) : m_value(upper, lower) { }
    constexpr uint128(int128<Endian> i128) : m_value(static_cast<uint64_t>(i128.m_value.upper()), i128.m_value.lower()) { }

    // Constructors from unsigned types
    constexpr uint128(uint64_t u) : m_value(0, u) { }
    constexpr uint128(uint32_t u) : uint128(static_cast<uint64_t>(u)) { }
    constexpr uint128(uint16_t u) : uint128(static_cast<uint64_t>(u)) { }
    constexpr uint128(uint8_t u) : uint128(static_cast<uint64_t>(u)) { }

    // Constructors from signed types
    constexpr uint128(int64_t i) : m_value(i < 0 ? std::numeric_limits<uint64_t>::max() : 0, i) { }
    constexpr uint128(int32_t i) : uint128(static_cast<int64_t>(i)) { }
    constexpr uint128(int16_t i) : uint128(static_cast<int64_t>(i)) { }
    constexpr uint128(int8_t i) : uint128(static_cast<int64_t>(i)) { }

    // Constructors from floating types
    constexpr uint128(float f) : uint128(make_uint128_from_float(f)) { }
    constexpr uint128(double d) : uint128(make_uint128_from_float(d)) { }
    constexpr uint128(long double ld) : uint128(make_uint128_from_float(ld)) { }
    
    // Assignment operator from arithmetic types
    uint128& operator=(uint128 rhs)
    { 
        m_value = rhs.m_value; 
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
    constexpr explicit operator bool() const { return m_value.lower() || m_value.upper(); }
    
    constexpr explicit operator char() const { return static_cast<char>(m_value.lower()); }
    constexpr explicit operator wchar_t() const { return static_cast<wchar_t>(m_value.lower()); }

    constexpr explicit operator char8_t() const { return static_cast<char8_t>(m_value.lower()); }
    constexpr explicit operator char16_t() const { return static_cast<char16_t>(m_value.lower()); }
    constexpr explicit operator char32_t() const { return static_cast<char32_t>(m_value.lower()); }

    constexpr explicit operator int8_t() const { return static_cast<int8_t>(m_value.lower()); }
    constexpr explicit operator int16_t() const { return static_cast<int16_t>(m_value.lower()); }
    constexpr explicit operator int32_t() const { return static_cast<int32_t>(m_value.lower()); }
    constexpr explicit operator int64_t() const { return static_cast<int64_t>(m_value.lower()); }

    constexpr explicit operator int128<Endian>() const 
    { 
        return int128<Endian>(
            static_cast<int64_t>(m_value.upper()),
            m_value.lower()
        );
    }

    constexpr explicit operator uint8_t() const { return static_cast<uint8_t>(m_value.lower()); }
    constexpr explicit operator uint16_t() const { return static_cast<uint16_t>(m_value.lower()); }
    constexpr explicit operator uint32_t() const { return static_cast<uint32_t>(m_value.lower()); }
    constexpr explicit operator uint64_t() const { return static_cast<uint64_t>(m_value.lower()); }

    constexpr explicit operator float() const { return make_float_from_uint128<float>(); }
    constexpr explicit operator double() const { return make_float_from_uint128<double>(); }
    constexpr explicit operator long double() const { return make_float_from_uint128<long double>(); }

    // Unary operators
    constexpr uint128 operator+() const { return *this; }
    
    constexpr uint128 operator-() const 
    {
        const auto hi = ~m_value.upper() + static_cast<uint64_t>(m_value.lower() == 0);
        const auto lo = ~m_value.lower() + 1;
        return uint128(hi, lo);
    }

    constexpr bool operator!() const { return !static_cast<bool>(*this); }

    constexpr uint128 operator~() const { return uint128(~m_value.upper(), ~m_value.lower()); }

    // Binary operators
    constexpr uint128 operator|(uint128 rhs) const { return bit_op(std::bit_or<>(), *this, rhs); }
    constexpr uint128 operator&(uint128 rhs) const { return bit_op(std::bit_and<>(), *this, rhs); }
    constexpr uint128 operator^(uint128 rhs) const { return bit_op(std::bit_xor<>(), *this, rhs); }

    constexpr uint128 operator+(uint128 rhs) const
    {
        const auto lo = m_value.lower() + rhs.m_value.lower();
        const auto carry = (lo < m_value.lower() ? 1 : 0);
        const auto hi = m_value.upper() + rhs.m_value.upper() + carry;
        return uint128(hi, lo);
    }

    constexpr uint128 operator-(uint128 rhs) const
    {
        const auto lo = m_value.lower() - rhs.m_value.lower();
        const auto carry = (lo <= m_value.lower() ? 0 : 1);
        const auto hi = m_value.upper() - rhs.m_value.upper() - carry;
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

        const uint64_t ah = m_value.lower() >> 32;
        const uint64_t al = m_value.lower() & mask;

        const uint64_t bh = rhs.m_value.lower() >> 32;
        const uint64_t bl = rhs.m_value.lower() & mask;
        
        const auto part_hi = m_value.upper() * rhs.m_value.lower() // H64(A) * L64(B)
                           + m_value.lower() * rhs.m_value.upper() // L64(A) * H64(B)
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
        
        const auto hi = m_value.upper();
        const auto lo = m_value.lower();

        return amount >= 64 
             ? uint128(lo << (amount - 64), 0)
             : uint128((hi << amount) | (lo >> (64 - amount)), lo << amount);
    }

    constexpr uint128 operator>>(int amount) const
    {
        assert(0 < amount && amount < 128 && "");

        const auto hi = m_value.upper();
        const auto lo = m_value.lower();

        return amount >= 64 
             ? uint128(0, hi >> (amount - 64)) 
             : uint128(hi >> amount, (lo >> amount) | (hi << (64 - amount)));
    }

    // Comparision
    constexpr bool operator==(uint128 rhs) const 
    { 
        return m_value.lower() == rhs.m_value.lower()
            && m_value.upper() == rhs.m_value.upper();
    }

    constexpr auto operator<=>(uint128 rhs) const
    {
        return m_value.upper() != rhs.m_value.upper()
            ? m_value.upper() <=> rhs.m_value.upper()
            : m_value.lower() <=> rhs.m_value.lower();
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
    constexpr int popcount() const
    {
        return std::popcount(m_value.lower()) 
             + std::popcount(m_value.upper());
    }

    constexpr bool has_single_bit() const { return popcount() == 1; }

    constexpr int countl_zero() const
    {
        return m_value.upper() == 0 
             ? std::countl_zero(m_value.lower()) + 64
             : std::countl_zero(m_value.upper());
    } 

    constexpr int countr_zero() const
    {
        return m_value.lower() == 0
             ? std::countr_zero(m_value.upper()) + 64
             : std::countr_zero(m_value.lower());
    }

    constexpr int countl_one() const
    {
        return m_value.upper() == std::numeric_limits<uint64_t>::max()
             ? std::countl_one(m_value.lower()) + 64
             : std::countl_one(m_value.upper());
    }

    constexpr int countr_one() const
    {
        return m_value.lower() == std::numeric_limits<uint64_t>::max()
             ? std::countr_one(m_value.upper()) + 64
             : std::countr_one(m_value.lower());
    }

    std::string to_string() const
    {
        std::bitset<64> b1(m_value.upper()), b2(m_value.lower());
        return b1.to_string() + b2.to_string();
    }

    static constexpr uint128 max() 
    { 
        return uint128(
            std::numeric_limits<uint64_t>::max(), 
            std::numeric_limits<uint64_t>::max()
        );
    }

private:

    Endian m_value;
};

template class uint128<>;

using uint128_t = uint128<>;

}

template <>
struct std::is_unsigned<leviathan::math::uint128_t> : std::true_type { };

template <>
struct std::numeric_limits<leviathan::math::uint128_t>
{
private:
    using uint128 = leviathan::math::uint128_t;

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

    static constexpr uint128 min() { return 0; }
    static constexpr uint128 lowest() { return 0; }
    static constexpr uint128 max() { return uint128::max(); }
    static constexpr uint128 epsilon() { return 0; }
    static constexpr uint128 round_error() { return 0; }
    static constexpr uint128 infinity() { return 0; }
    static constexpr uint128 quiet_NaN() { return 0; }
    static constexpr uint128 signaling_NaN() { return 0; }
    static constexpr uint128 denorm_min() { return 0; }
};


namespace leviathan::math
{

template <typename Endian>
class int128
{
    friend class uint128<Endian>;
public:

    constexpr int128() = default;
    constexpr int128(const int128&) = default;
    constexpr int128(typename Endian::upper_type upper, typename Endian::lower_type lower) : m_value(upper, lower) { }
    constexpr int128(uint128<Endian> u128) : m_value(static_cast<int64_t>(u128.m_value.upper()), u128.m_value.lower()) { }

    // COnstructors from unsigned types
    constexpr int128(uint64_t u);
    constexpr int128(uint32_t u);
    constexpr int128(uint16_t u);
    constexpr int128(uint8_t u);

    // Constructors from signed types
    constexpr int128(int64_t i);
    constexpr int128(int32_t i);
    constexpr int128(int16_t i);
    constexpr int128(int8_t i);

    // Constructors from floating types
    constexpr int128(float f);
    constexpr int128(double d);
    constexpr int128(long double ld);

    int128& operator=(int128 rhs)
    {
        m_value = rhs.m_value;
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
    constexpr explicit operator bool() const;
    
    constexpr explicit operator char() const;
    constexpr explicit operator wchar_t() const;

    constexpr explicit operator char8_t() const;
    constexpr explicit operator char16_t() const;
    constexpr explicit operator char32_t() const;

    constexpr explicit operator int8_t() const;
    constexpr explicit operator int16_t() const;
    constexpr explicit operator int32_t() const;
    constexpr explicit operator int64_t() const;

    constexpr explicit operator uint8_t() const;
    constexpr explicit operator uint16_t() const;
    constexpr explicit operator uint32_t() const;
    constexpr explicit operator uint64_t() const;

    constexpr explicit operator uint128<Endian>() const;

    constexpr explicit operator float() const;
    constexpr explicit operator double() const;
    constexpr explicit operator long double() const;

    // Unary operators
    constexpr int128 operator+() const { return *this; }

    constexpr int128 operator-() const;

    constexpr bool operator!() const;

    constexpr int128 operator~() const;

    // Binary operators
    constexpr int128 operator|(int128 rhs) const { return bit_op(std::bit_or<>(), *this, rhs); }
    constexpr int128 operator&(int128 rhs) const { return bit_op(std::bit_and<>(), *this, rhs); }
    constexpr int128 operator^(int128 rhs) const { return bit_op(std::bit_xor<>(), *this, rhs); }

    constexpr int128 operator+(int128 rhs) const;
    constexpr int128 operator-(int128 rhs) const;
    constexpr int128 operator*(int128 rhs) const;
    constexpr int128 operator/(int128 rhs) const;
    constexpr int128 operator%(int128 rhs) const;
    constexpr int128 operator<<(int128 rhs) const;
    constexpr int128 operator>>(int128 rhs) const;

    constexpr bool operator==(int128 rhs) const;
    constexpr auto operator<=>(int128 rhs) const;

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

private:

    Endian m_value;
};

}