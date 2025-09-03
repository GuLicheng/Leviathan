#pragma once

#include <leviathan/config_parser/context.hpp>
#include <leviathan/lang/rust/rust.hpp>

namespace nom
{
    
using cpp::config::context;

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
};

// Difference from nom::ErrorKind, we make ErrorCode a template parameter.
template <typename Input, typename ErrorCode>
struct error
{
    Input input;
    ErrorCode code;
    bool recoverable = true;  // Failure for rust::nom is unrecoverable by default.
};

template <typename Input, typename Output, typename Error = error<Input, error_kind>>
class iresult : public rust::result<std::pair<Input, Output>, Error>
{
    using base = rust::result<std::pair<Input, Output>, Error>;

public:

    using input_type = Input;
    using output_type = Output;
    using typename base::error_type;

    template <typename... Args>
    constexpr iresult(rust::in_place_t, Args&&... args)
        : base(rust::in_place,  (Args&&)args...)
    { }

    template <typename... Args>
    constexpr iresult(rust::unexpect_t, Args&&... args)
        : base(rust::unexpect, (Args&&)args...)
    { }

};

} // namespace nom




