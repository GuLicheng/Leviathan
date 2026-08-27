#pragma once

#include "error_kind.hpp"

#include <leviathan/extc++/annotation.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/expected.hpp>

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
class err
{
    // >_< !
    // struct [[=cpp::derive::debug]] incomplete { size_t value; };
    // struct [[=cpp::derive::debug]] recoverable { Recoverable value; };
    // struct [[=cpp::derive::debug]] unrecoverable { Unrecoverable value; };

    template <size_t N, typename... Args>
    constexpr err(std::in_place_index_t<N>, Args&&... args) : value(std::in_place_index<N>, std::forward<Args>(args)...) { }

public:

    using incomplete = size_t;

    using incomplete_type = incomplete;
    using recoverable_type = Recoverable;
    using unrecoverable_type = Unrecoverable;

    constexpr err(const err&) = default;
    constexpr err(err&&) = default;

    static constexpr err make_incomplete(size_t n) { return err(std::in_place_index<0>, n); }

    template <typename... Args>
    static constexpr err make_recoverable(Args&&... args) 
    { 
        return err(std::in_place_index<1>, std::forward<Args>(args)...); 
    }
    
    template <typename... Args>
    static constexpr err make_unrecoverable(Args&&... args) 
    { 
        return err(std::in_place_index<2>, std::forward<Args>(args)...); 
    }

    // We use index instead of std::holds_alternative to avoid the overhead of typeid.
    constexpr bool is_incomplete() const { return value.index() == 0; }
    constexpr bool is_recoverable() const { return value.index() == 1; }
    constexpr bool is_unrecoverable() const { return value.index() == 2; }

    constexpr auto& as_incomplete() const { return std::get<0>(value); }
    constexpr auto& as_incomplete() { return std::get<0>(value); }
    
    constexpr auto& as_recoverable() { return std::get<1>(value); }
    constexpr auto& as_recoverable() const { return std::get<1>(value); }

    constexpr auto& as_unrecoverable() { return std::get<2>(value); }
    constexpr auto& as_unrecoverable() const { return std::get<2>(value); }


private:

    std::variant<incomplete, Recoverable, Unrecoverable> value;

};

template <typename Input, typename ErrorCode = error_kind>
struct error
{
    Input input;
    ErrorCode code;
};

} // namespace nom

template <typename Recoverable, typename Unrecoverable>
struct std::formatter<nom::err<Recoverable, Unrecoverable>>
{
    template <typename FormatContext>
    static constexpr auto parse(FormatContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    static auto format(const nom::err<Recoverable, Unrecoverable>& e, FormatContext& ctx)
    {
        if (e.is_incomplete())
        {
            return std::format_to(ctx.out(), "Incomplete({})", e.as_incomplete());
        }
        else if (e.is_recoverable())
        {
            return std::format_to(ctx.out(), "Recoverable({})", e.as_recoverable());
        }
        else
        {
            return std::format_to(ctx.out(), "Unrecoverable({})", e.as_unrecoverable());
        }
    }
};

template <typename Input, typename ErrorCode>
struct std::formatter<nom::error<Input, ErrorCode>>
{
    template <typename FormatContext>
    static constexpr auto parse(FormatContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    static auto format(const nom::error<Input, ErrorCode>& e, FormatContext& ctx)
    {
        return std::format_to(ctx.out(), "input: {}, code: {}", e.input, e.code);
    }
};

