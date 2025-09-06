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
    many1,
    unknown,
};

/**
 * @brief The error structure for nom parsers.
 * 
 * The error in Rust::nom has three states: Incomplete, Error, Failure.
 * 
 * @param Incomplete Indicating how many characters we still need in input.
 * @param Error A recoverable error tag, we can try other parsers.
 * @param Failure An unrecoverable error, we should stop parsing.
 * 
 * Here is the definition of nom::Err in Rust:
 * 
 *  pub enum Err<Failure, Error = Failure> {
 *    Incomplete(Needed),
 *    Error(Error),
 *    Failure(Failure),
 *  }
 *  
 *  We have found that in most cases, the Error and Failure are the same.
 *  So we merge them into one, and use a boolean flag to indicate 
 *  whether it is recoverable.
 */
template <typename Input, typename ErrorCode>
struct error
{
    Input input;
    ErrorCode code;
    bool recoverable = true;  
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




