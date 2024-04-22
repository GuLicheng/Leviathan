// https://github.com/abseil/abseil-cpp/blob/master/absl/numeric/int128.h
// https://github.com/dotnet/runtime/blob/5535e31a712343a63f5d7d796cd874e563e5ac14/src/libraries/System.Private.CoreLib/src/System/UInt128.cs
#pragma once

#include "common.hpp"

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

    constexpr static bool lower_index = (Endian == std::endian::little);

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

public:

    constexpr uint128() = default;
    constexpr uint128(const uint128&) = default;
    constexpr uint128(typename Endian::upper_type upper, typename Endian::lower_type lower) : m_value(upper, lower) { }

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
    constexpr uint128(long double ld) : uint128(make_uint128_from_float(ld)) { }
    constexpr uint128(float f) : uint128(make_uint128_from_float(f)) { }
    constexpr uint128(double d) : uint128(make_uint128_from_float(d)) { }
    
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

    constexpr uint128& operator=(long double ld) { return *this = uint128(ld); }
    constexpr uint128& operator=(float f) { return *this = uint128(f); }
    constexpr uint128& operator=(double d) { return *this = uint128(d); }

    // Conversion operators to other arithmetic types
    constexpr explicit operator bool() const { return m_value.lower() || m_value.upper(); }
    
    constexpr explicit operator char() const { return static_cast<char>(m_value.lower()); }
    constexpr explicit operator char16_t() const { return static_cast<char16_t>(m_value.lower()); }
    constexpr explicit operator char32_t() const { return static_cast<char32_t>(m_value.lower()); }

    constexpr explicit operator int8_t() const { return static_cast<int8_t>(m_value.lower()); }
    constexpr explicit operator int16_t() const { return static_cast<int16_t>(m_value.lower()); }
    constexpr explicit operator int32_t() const { return static_cast<int32_t>(m_value.lower()); }
    constexpr explicit operator int64_t() const { return static_cast<int64_t>(m_value.lower()); }

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

    constexpr bool operator!() const { return !m_value.upper() && !m_value.lower(); }

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
        constexpr uint64_t mask = 0xffffffff;   // mask low 64-bit

        const uint64_t ah = m_value.lower() >> 32;
        const uint64_t al = m_value.lower() & mask;

        const uint64_t bh = rhs.m_value.lower() >> 32;
        const uint64_t bl = rhs.m_value.lower() & mask;
        
        const auto part_hi = m_value.upper() * rhs.m_value.lower() 
                           + m_value.lower() * rhs.m_value.upper() 
                           + ah * bh;

        const auto lo = al * bl;

        uint128 result(part_hi, lo);

        result += uint128(ah * bl) << 32;
        result += uint128(bh * al) << 32;

        return result;
    }

    constexpr uint128 operator/(uint128 rhs) const { throw 0; }
    constexpr uint128 operator%(uint128 rhs) const { throw 0; }

    constexpr uint128 operator<<(int value) const
    {
        assert(0 < value && value < 128 && "");
        
        const auto hi = m_value.upper();
        const auto lo = m_value.lower();

        return value >= 64 
             ? uint128(lo << (value - 64), 0)
             : uint128((hi << value) | (lo >> (64 - value)), lo << value);
    }

    constexpr uint128 operator>>(int value) const
    {
        assert(0 < value && value < 128 && "");

        const auto hi = m_value.upper();
        const auto lo = m_value.lower();

        return value >= 64 
             ? uint128(0, hi >> (value - 64)) 
             : uint128(hi >> value, (lo >> value) | (hi << (64 - value)));
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

    constexpr uint128& operator+=(uint128 rhs) { return *this = *this + rhs; }
    constexpr uint128& operator-=(uint128 rhs) { return *this = *this - rhs; }
    constexpr uint128& operator*=(uint128 rhs) { return *this = *this * rhs; }
    constexpr uint128& operator/=(uint128 rhs) { return *this = *this / rhs; }
    constexpr uint128& operator%=(uint128 rhs) { return *this = *this % rhs; }
    constexpr uint128& operator|=(uint128 rhs) { return *this = *this | rhs; }
    constexpr uint128& operator^=(uint128 rhs) { return *this = *this ^ rhs; }
    constexpr uint128& operator&=(uint128 rhs) { return *this = *this & rhs; }

    constexpr uint128& operator<<=(int value) { return *this = *this >> value; }
    constexpr uint128& operator>>=(int value) { return *this = *this << value; }

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

    constexpr friend int popcount(uint128 x) 
    {
        return std::popcount(x.m_value.lower()) 
             + std::popcount(x.m_value.upper());
    }

    std::string to_string() const
    {
        std::bitset<64> b1(m_value.upper()), b2(m_value.lower());
        return b1.to_string() + b2.to_string();
    }

    constexpr static uint128 max() 
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