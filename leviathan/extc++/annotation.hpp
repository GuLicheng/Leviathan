#pragma once

#include <algorithm>

namespace cpp::derive
{

// Extend std::formatter for class or enum type
inline constexpr struct { } debug;

// Extend std::hash for class or enum type
inline constexpr struct { } hash;

// Allow a class to be serialized to a specific type, for example, json::value.
// Extend optional_cast<StructOrEnum, T> 
template <typename T> struct from_t { explicit from_t() = default; };
template <typename T> inline constexpr auto from = from_t<T>{};

// Allow a class to be deserialized from a specific type, for example, json::value.
// Extend optional_cast<T, StructOrEnum> 
template <typename T> struct into_t { explicit into_t() = default; };
template <typename T> inline constexpr auto into = into_t<T>{};

// Extend operator| and operator|= for enum type, for example, to support bitmask operations.
inline constexpr struct { } op_pipe;

// Extend std::tuple_size and std::tuple_element and std::get
// for class type, to allow structured bindings and tuple-like interface.
inline constexpr struct { } tuple_like;

}  // namespace cpp::derive


namespace cpp::refl
{

inline constexpr struct { } tuple_element;

inline constexpr struct { } choice_annotation;

inline constexpr struct { } skip;

inline constexpr struct { } skip_serialization;

inline constexpr struct { } skip_deserialization;

inline constexpr struct { } rename_annotation;

inline constexpr struct { } value_annotation;

inline constexpr struct { } parse_annotation;

/**
 * @brief Check if the given annotation is present on the given info.
 * @param r Anything that can be reflected, such as class, field, base class, etc.
 * @param obj The annotation to check.
 * @return true if the annotation is present, false otherwise.
 * @example 
 * struct SomeThing { [[=some_annotation]] int x; }
 * static_assert(has_annotation(^^SomeThing::x, some_annotation));
 */
template <typename... Ts>
consteval bool has_annotation(std::meta::info r, const Ts&... objs) 
{
    return (... || std::ranges::contains(
        annotations_of_with_type(r, ^^Ts),
        std::meta::reflect_constant(objs),
        std::meta::constant_of
    ));
}

}  // namespace cpp::refl