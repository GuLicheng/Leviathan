#pragma once

#include "error_kind.hpp"

#include <leviathan/extc++/annotation.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/extc++/format.hpp>

namespace nom
{

template <typename T, typename E>
class [[=cpp::derive::debug]] result
{
    struct [[=cpp::derive::debug]] ok { T value; };
    struct [[=cpp::derive::debug]] err { E value; };

    constexpr result(ok o) : value(std::in_place_index<0>, std::move(o)) { }
    constexpr result(err e) : value(std::in_place_index<1>, std::move(e)) { }

public:

    constexpr result(const result&) = default;
    constexpr result(result&&) = default;

    static constexpr result make_ok(T t) { return result(ok{ .value = std::move(t) }); }
    static constexpr result make_err(E e) { return result(err{ .value = std::move(e) }); }

    constexpr bool is_ok() const { return std::holds_alternative<ok>(value); }
    constexpr bool is_err() const { return std::holds_alternative<err>(value); }

    template <typename Self>
    constexpr auto& unwrap_ok(this Self&& self) 
    { 
        return std::forward_like<Self>(std::get<ok>(self.value).value);
    }

    template <typename Self>
    constexpr auto& unwrap_err(this Self&& self)
    { 
        return std::forward_like<Self>(std::get<err>(self.value).value);
    }

private:

    std::variant<ok, err> value;

};

/**
 * @brief The error structure for nom parsers.
 * https://docs.rs/nom/latest/nom/error/struct.Error.html
 * 
 * The error in Rust::nom has three states: Incomplete, Error and Failure.
 * 
 * @param Incomplete Indicating how many characters we still need in input.
 * @param Error A recoverable error tag, we can try other parsers.
 * @param Failure An unrecoverable error, we should stop parsing.
 * 
 * Here is the definition of Err in Rust: https://docs.rs/nom/latest/nom/enum.Err.html
 * 
 *  pub enum Err<Failure, Error = Failure> {
 *    Incomplete(Needed),
 *    Error(Error),
 *    Failure(Failure),
 *  }
 * 
 *  pub enum Needed {
 *    Unknown,
 *    Size(NonZeroUsize),
 *  }
 * 
 * The Incomplete in our error can just be a size_t, 
 * indicating how many characters we still need in input.
 */
template <typename Failure, typename Error = Failure>
class [[=cpp::derive::debug]] err
{
    // >_< !
    struct [[=cpp::derive::debug]] incomplete { size_t value; };
    struct [[=cpp::derive::debug]] error { Error value; };
    struct [[=cpp::derive::debug]] failure { Failure value; };

    constexpr err(incomplete i) : value(std::in_place_index<0>, std::move(i)) { }
    constexpr err(error e) : value(std::in_place_index<1>, std::move(e)) { }
    constexpr err(failure f) : value(std::in_place_index<2>, std::move(f)) { }

public:

    constexpr err(const err&) = default;
    constexpr err(err&&) = default;

    static constexpr err make_incomplete(size_t n) { return err(incomplete{ .value = n }); }
    static constexpr err make_error(Error e) { return err(error{ .value = std::move(e) }); }
    static constexpr err make_failure(Failure f) { return err(failure{ .value = std::move(f) }); }

    constexpr bool is_incomplete() const { return std::holds_alternative<incomplete>(value); }
    constexpr bool is_error() const { return std::holds_alternative<error>(value); }
    constexpr bool is_failure() const { return std::holds_alternative<failure>(value); }

    constexpr auto& as_incomplete() { return std::get<incomplete>(value); }
    constexpr auto& as_error() { return std::get<error>(value); }
    constexpr auto& as_failure() { return std::get<failure>(value); }

private:

    std::variant<incomplete, error, failure> value;

};

template <typename I, typename O, typename E>
using iresult = result<std::pair<I, O>, err<E>>;

// // https://docs.rs/nom/latest/nom/error/trait.ParseError.html
// template <typename E, typename I>
// concept parse_error = requires(E err, I input, error_kind kind, typename I::context_type ctx) 
// {
//     { E::from_error_kind(input, kind) } -> std::same_as<E>;

//     { err.append(input, kind) } -> std::same_as<void>;

//     { err.add_context(input, ctx) } -> std::same_as<void>;
// };

} // namespace nom


