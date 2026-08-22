#pragma once

namespace nom
{

// https://docs.rs/nom/latest/nom/error/enum.ErrorKind.html
enum class error_kind
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


}  // namespace nom
