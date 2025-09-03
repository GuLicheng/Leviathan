#pragma once

#include <leviathan/type_caster.hpp>
#include <expected>
#include <optional>
#include <tuple>

namespace rust
{
    
template <typename T, typename E>
using result = std::expected<T, E>;

// I am not sure how to map `()` in rust to C++.
// Maybe use std::tuple<> or define an empty struct?
// https://doc.rust-lang.org/nightly/std/primitive.unit.html
// It seems like `void` in C++ but a complete type.
using unit = std::tuple<>;

using in_place_t = std::in_place_t;
using std::nullopt_t;
using std::unexpect_t;

using std::in_place;
using std::unexpect;
using std::nullopt;

} // namespace rust

