#pragma once

#include <leviathan/extc++/enum.hpp>

namespace nom
{

// https://docs.rs/nom/latest/nom/error/enum.ErrorKind.html
enum class [[=cpp::derive::debug]] error_kind
{
    ok,
    tag,
    take_while1,
    take_till1,
    is_a,
    is_not,
    eof,
    digit,
    alpha,
    space,
    multispace,
    alphanumeric,
    one_of,
    none_of,
    satisfy,
    one_char,
    bin_digit,
    oct_digit,
    hex_digit,
    crlf,
    alt,
    fail,
    not_,
    verify,
    many1,
    unknown,
};

template <typename ErrorCode>
struct error_traits
{
    // Convert an error_kind to the corresponding ErrorCode type.
    // E.g. ErrorCode is std::string, we can convert error_kind::tag to "tag" string.
    static constexpr ErrorCode from_error_kind(error_kind kind) = delete;

    // Append an error_kind to an existing ErrorCode. E.g. ErrorCode is std::string, 
    // we can append "tag" to the existing error message.
    template <typename Context>
    static constexpr ErrorCode append(const Context& ctx, error_kind kind, const ErrorCode& other) = delete;

    template <typename Context>
    static constexpr ErrorCode add_context(const Context& ctx, const char* context, const ErrorCode& other) = delete;
};

template <>
struct error_traits<error_kind>
{
    static constexpr error_kind from_error_kind(error_kind kind) { return kind; }

    template <typename Context>
    static constexpr error_kind append(const Context& ctx, error_kind kind, const error_kind& other) { return kind; }

    // template <typename Context>
    // static constexpr error_kind add_context(const Context& ctx, const char* context, const error_kind& other) { return other; }
};


}  // namespace nom


