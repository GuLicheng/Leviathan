#pragma once

#include "error_kind.hpp"

#include <leviathan/extc++/annotation.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/extc++/format.hpp>

namespace nom
{

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
 * We make the Incomplete in our error structure a size_t,
 * and rename the Error and Failure to Recoverable and Unrecoverable.
 */
template <typename Recoverable, typename Unrecoverable = Recoverable>
class [[=cpp::derive::debug]] error
{
    // >_< !
    struct [[=cpp::derive::debug]] incomplete { size_t value; };
    struct [[=cpp::derive::debug]] recoverable { Recoverable value; };
    struct [[=cpp::derive::debug]] unrecoverable { Unrecoverable value; };

    constexpr error(incomplete i) : value(std::in_place_index<0>, std::move(i)) { }
    constexpr error(recoverable e) : value(std::in_place_index<1>, std::move(e)) { }
    constexpr error(unrecoverable f) : value(std::in_place_index<2>, std::move(f)) { }

    template <size_t N, typename T>
    constexpr error(std::in_place_index_t<N>, T&& v) : value(std::in_place_index<N>, std::forward<T>(v)) { }

public:

    using incomplete_type = incomplete;
    using recoverable_type = Recoverable;
    using unrecoverable_type = Unrecoverable;

    constexpr error(const error&) = default;
    constexpr error(error&&) = default;

    static constexpr error make_incomplete(size_t n) { return error(incomplete{ .value = n }); }
    static constexpr error make_recoverable(Recoverable e) { return error(recoverable{ .value = std::move(e) }); }
    static constexpr error make_unrecoverable(Unrecoverable f) { return error(unrecoverable{ .value = std::move(f) }); }

    constexpr bool is_incomplete() const { return std::holds_alternative<incomplete>(value); }
    constexpr bool is_recoverable() const { return std::holds_alternative<recoverable>(value); }
    constexpr bool is_unrecoverable() const { return std::holds_alternative<unrecoverable>(value); }

    constexpr auto& as_incomplete() { return std::get<incomplete>(value); }
    constexpr auto& as_recoverable() { return std::get<recoverable>(value); }
    constexpr auto& as_unrecoverable() { return std::get<unrecoverable>(value); }

private:

    std::variant<incomplete, recoverable, unrecoverable> value;

};

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
    constexpr auto&& unwrap_ok(this Self&& self) 
    { 
        return std::forward_like<Self>(std::get<ok>(self.value).value);
    }

    template <typename Self>
    constexpr auto&& unwrap_err(this Self&& self)
    { 
        return std::forward_like<Self>(std::get<err>(self.value).value);
    }

    template <typename Self, typename F>
    constexpr auto map_err(this Self&& self, F&& f)
    {
        using Recoverable = std::invoke_result_t<F, typename E::recoverable>;
        using Unrecoverable = std::invoke_result_t<F, typename E::unrecoverable>;
        
        using Err = error<Recoverable, Unrecoverable>;
        using R = result<T, Err>;

        if (self.is_ok())
        {
            return R::make_ok(std::forward_like<Self>(self).unwrap_ok());
        }
        
        auto&& e = self.unwrap_err();

        if (e.is_incomplete())
        {
            return R::make_err(Err::make_incomplete(e.as_incomplete().value));
        }
        else if (e.is_recoverable())
        {
            return R::make_err(Err::make_recoverable(std::invoke(std::forward<F>(f), std::forward_like<Self>(e).as_recoverable().value)));
        }
        else
        {
            return R::make_err(Err::make_unrecoverable(std::invoke(std::forward<F>(f), std::forward_like<Self>(e).as_unrecoverable().value)));
        }
    }

private:

    std::variant<ok, err> value;

};


template <typename I, typename O, typename E>
using iresult = result<std::pair<I, O>, error<E>>;

// // https://docs.rs/nom/latest/nom/error/trait.ParseError.html
// template <typename E, typename I>
// concept parse_error = requires(E err, I input, error_kind kind, typename I::context_type ctx) 
// {
//     { E::from_error_kind(input, kind) } -> std::same_as<E>;

//     { err.append(input, kind) } -> std::same_as<void>;

//     { err.add_context(input, ctx) } -> std::same_as<void>;
// };

} // namespace nom


