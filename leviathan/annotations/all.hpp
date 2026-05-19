/*
    Follow annotations are just tags that can help us to generate code or 
    provide metadata for reflection, they don't have any logic by themselves, 
    but we can write code to interpret them.
*/

#pragma once

#include <leviathan/annotations/rename.hpp>
#include <leviathan/annotations/parser.hpp>
#include <leviathan/annotations/choice.hpp>

namespace cpp::derive
{

// Extend std::formatter for class or enum type
inline constexpr struct { } debug;

// Extend std::hash for class or enum type
inline constexpr struct { } hash;

// Allow a class to be serialized to a specific type, for example, json::value.
template <typename T> struct encode_t { };
template <typename T> inline constexpr auto encode = encode_t<T>{};

// Allow a class to be deserialized from a specific type, for example, json::value.
template <typename T> struct decode_t { };
template <typename T> inline constexpr auto decode = decode_t<T>{};

// Extend operator| and operator|= for enum type, for example, to support bitmask operations.
inline constexpr struct { } op_pipe;

}