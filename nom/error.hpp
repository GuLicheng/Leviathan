#pragma once

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
    // struct [[=cpp::derive::debug]] incomplete { size_t value; };
    // struct [[=cpp::derive::debug]] recoverable { Recoverable value; };
    // struct [[=cpp::derive::debug]] unrecoverable { Unrecoverable value; };

    template <size_t N, typename T>
    constexpr error(std::in_place_index_t<N>, T&& v) : value(std::in_place_index<N>, std::forward<T>(v)) { }

public:

    using incomplete = size_t;

    using incomplete_type = incomplete;
    using recoverable_type = Recoverable;
    using unrecoverable_type = Unrecoverable;

    constexpr error(const error&) = default;
    constexpr error(error&&) = default;

    static constexpr error make_incomplete(size_t n) { return error(std::in_place_index<0>, n); }
    static constexpr error make_recoverable(Recoverable e) { return error(std::in_place_index<1>, std::move(e)); }
    static constexpr error make_unrecoverable(Unrecoverable f) { return error(std::in_place_index<2>, std::move(f)); }

    // We use index instead of std::holds_alternative to avoid the overhead of typeid.
    constexpr bool is_incomplete() const { return value.index() == 0; }
    constexpr bool is_recoverable() const { return value.index() == 1; }
    constexpr bool is_unrecoverable() const { return value.index() == 2; }

    constexpr auto& as_incomplete() { return std::get<0>(value); }
    constexpr auto& as_recoverable() { return std::get<1>(value); }
    constexpr auto& as_unrecoverable() { return std::get<2>(value); }

private:

    std::variant<incomplete, Recoverable, Unrecoverable> value;

};

} // namespace nom


