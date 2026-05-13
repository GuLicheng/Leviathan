/*
    Follow annotations are just tags that can help us to generate code or 
    provide metadata for reflection, they don't have any logic by themselves, 
    but we can write code to interpret them.
*/

#pragma once

#include <leviathan/annotations/rename.hpp>
#include <leviathan/annotations/parser.hpp>

namespace cpp::derive
{

inline constexpr struct { } debug;

inline constexpr struct { } hash;

inline constexpr struct { } serialize;

inline constexpr struct { } deserialize;

inline constexpr struct { } skip;

inline constexpr struct { } required;

}