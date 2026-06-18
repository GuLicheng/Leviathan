#pragma once

#include <algorithm>

namespace cpp::derive
{

/**
 * @brief Allow a class or enum type to be formatted with std::format. 
 */
inline constexpr struct { } debug;

/**
 * @brief Allow a class or enum type to be hashed with std::hash.
 */
inline constexpr struct { } hash;

/**
 * @brief Allow a class or enum to be converted into another 
 * type, for example, json::value.
 */
template <typename T> struct from_t { explicit from_t() = default; };
template <typename T> inline constexpr auto from = from_t<T>{};

/**
 * @brief Allow a class or enum to be converted from another 
 * type, for example, json::value.
 */
template <typename T> struct into_t { explicit into_t() = default; };
template <typename T> inline constexpr auto into = into_t<T>{};

/**
 * @brief Allow an enum type to support operator| and operator|=, 
 * which is useful for bitmask operations.
 */
inline constexpr struct { } op_pipe;

/**
 * @brief Allow a class to be treated as a tuple-like type, 
 * which means that we can use structured bindings on it.
 */
inline constexpr struct { } tuple_like;

}  // namespace cpp::derive

namespace cpp::refl
{

// Any field annotated with [[=skip]] will be ignored in code generation
// When initializing a struct from a tuple, the skipped fields will be 
// initialized with default value or default initializer.
inline constexpr struct { } skip;

inline constexpr struct { } test;

// Any class annotated with [[=produces_range]] will be treated as a range producer, 
// such as derive from followe class
// class SomeInterface { std::ranges::range<R> operator()(); }
inline constexpr struct { } produces_range;

// Any field annotated with [[=value_guard]] will be treated as a choice field, which means that
// when initializing the field, we will try to find an annotation with [[=value_guard]] 
// and use it to check if the value is valid. Such as derive from followe class
// class SomeInterface { bool operator(const auto&); }
inline constexpr struct { } value_guard;

inline constexpr struct { } skip_serialization;

inline constexpr struct { } skip_deserialization;

// Rename annotaion
inline constexpr struct { } modify_identifier;

inline constexpr struct { } value;

inline constexpr struct { } parse_annotation;



}  // namespace cpp::refl