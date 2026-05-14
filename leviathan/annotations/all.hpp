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

inline constexpr struct { } debug;

inline constexpr struct { } hash;

template <typename T> struct deserialize_t { };

template <typename T> inline constexpr auto deserialize = deserialize_t<T>{};

template <typename T> struct serialize_t { };

template <typename T> inline constexpr auto serialize = serialize_t<T>{};

inline constexpr struct { } skip;

inline constexpr struct { } required;

}