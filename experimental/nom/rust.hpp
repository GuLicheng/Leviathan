#pragma once

#include <leviathan/type_caster.hpp>
#include <expected>

namespace rust
{
    
template <typename T, typename E>
using Result = std::expected<T, E>;

// I am not sure how to map `()` in rust to C++.
// Maybe use std::tuple<> or define an empty struct?
// https://doc.rust-lang.org/nightly/std/primitive.unit.html
// It seems like `void` in C++ but a complete type.
using Unit = std::tuple<>;

} // namespace rust

