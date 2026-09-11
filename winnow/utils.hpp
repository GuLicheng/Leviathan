#pragma once

#include <optional>
#include <ranges>
#include <iterator>
#include <contracts>

namespace winnow
{

/**
 * @brief Traits for parse error type, corresponds to winnow ParserError / AddContext / FromExternalError.
 * https://docs.rs/winnow/1.0.0/winnow/trait.ParserError.html
 *
 * @tparam E The concrete error payload type.
 */
template <typename E>
struct error_traits
{
    /**
     * @brief Create a base empty error from input stream. We remove the
     *   Rust::winnow::ParserError::assert and add another parameter 
     *   `message` for the error message.
     * 
     * @param stream Current parse input stream.
     * @param message Error message describing the context of the error.
     */
    template <typename Stream>
    static constexpr E from_input(const Stream& stream, const char* message);

    /**
     * @brief Append context description to existing error.
     * @param e The error object to which the context is being added.
     * @param stream Current parse input stream.
     * @param token_start The checkpoint in the input stream where the context is being added.
     * @param ctx Additional context information.
     */
    template <typename Stream, typename Context>
    static constexpr void add_context(E& e, Stream& stream, Context ctx);

    /**
     * @brief Convert external error (e.g. numeric parse fail) into parse error.
     * @tparam Stream Stream type satisfying stream concept.
     * @tparam Ext External error type.
     * @param stream Current parse input stream.
     * @param ext Original external error.
     */
    // template <typename Stream, typename Ext>
    // static constexpr E from_external(const Stream& stream, Ext&& ext);


    // fn append(self, _input: &I, _token_start: &<I as Stream>::Checkpoint) -> Self
    // template <typename Stream, typename Checkpoint>
    // static constexpr E append(E err, Stream& stream, const Checkpoint& checkpoint);

};

/**
 * @brief https://docs.rs/winnow/latest/winnow/stream/trait.Accumulate.html
 * 
 * pub trait Accumulate<T>: Sized {
 *     
 *     fn initial(capacity: Option<usize>) -> Self;
 * 
 *     fn accumulate(&mut self, acc: T);
 * }
 */
template <typename T>
struct accumulate_traits
{
    // We remove the capacity parameter as it is not used in the generic implementation.
    static constexpr T initial();

    template <typename U>
    static constexpr void accumulate(T& acc, U&& value);
};

template <std::ranges::range R>
struct accumulate_traits<R>
{
    static constexpr R initial()
    {
        return R();
    }

    template <typename Arg>
    static constexpr void accumulate(R& acc, Arg&& value)
    {
        using U = typename R::value_type;
        auto inserter = std::inserter(acc, acc.end());
        // For string_view may not convert to string automatically 
        U u = static_cast<U>(std::forward<Arg>(value));
        *inserter++ = std::move(u);
    }
};

// https://docs.rs/winnow/latest/winnow/stream/struct.Range.html
template <typename T = size_t>
struct occurrences
{
    T lower;
    std::optional<T> upper;

    constexpr occurrences(T l, std::optional<T> u) : lower(l), upper(u) 
    {
        // All offset ranges follow C++ half‑open convention 
        // [lower, upper): lower is included, upper is excluded.
        // contract_assert(lower < (upper ? *upper : lower));
    }

    constexpr occurrences(T l, T u) : occurrences(l, std::optional<T>(u)) { }

    explicit constexpr occurrences(T l) : occurrences(l, std::nullopt) { }

    constexpr bool contains(T value) const
    {
        // Check whether the value is within the range [lower, upper)
        return value >= lower && (!upper || value < *upper);
    }

    constexpr bool is_over(T value) const
    {
        return upper && value >= *upper;
    }

    // For [lower, upper), when size count reaches the upper bound(upper - 1)
    // we may stop accumulating further elements and parse the next part of the input.
    constexpr bool is_upper_bound(T value) const
    {
        return upper && (value + 1 == *upper);
    }

    constexpr bool is_under(T value) const
    {
        return value < lower;
    }
};

constexpr occurrences<size_t> range(size_t lower, size_t upper)
{
    return occurrences<size_t>(lower, upper);
}

constexpr occurrences<size_t> from(size_t lower)
{
    return occurrences<size_t>(lower, std::nullopt);
}

constexpr occurrences<size_t> upto(size_t upper)
{
    return range(0, upper);
}

using unit = std::tuple<>;


}  // namespace winnow






