#pragma once

#include <leviathan/extc++/variant.hpp>

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
     * @brief Create a base empty error from input stream.
     * @tparam Stream Stream type satisfying stream concept.
     * @param stream Current parse input stream.
     */
    template <stream Stream>
    static constexpr E from_input(const Stream& stream);

    /**
     * @brief Append context description to existing error.
     * @tparam Stream Stream type satisfying stream concept.
     * @tparam Item Context metadata type (like StrContext in winnow).
     * @param stream Current parse input stream.
     * @param err Existing error instance.
     * @param item Additional context information.
     */
    template <stream Stream, typename Item>
    static constexpr E add_context(const Stream& stream, E err, Item&& item);

    /**
     * @brief Convert external error (e.g. numeric parse fail) into parse error.
     * @tparam Stream Stream type satisfying stream concept.
     * @tparam Ext External error type.
     * @param stream Current parse input stream.
     * @param ext Original external error.
     */
    template <stream Stream, typename Ext>
    static constexpr E from_external(const Stream& stream, Ext&& ext);
};


/**
 * @brief The error structure for winnow parsers.
 * https://docs.rs/winnow/1.0.0/winnow/error/enum.ErrMode.html
 * 
 * @tparam Error The error type for Backtrack and Cut.
 * @tparam Incomplete The error type for Incomplete, default to size_t.
 * 
 *  pub enum ErrMode<E> {
 *     Incomplete(Needed),
 *     Backtrack(E),
 *     Cut(E),
 *  }
 *  
 *  Since the Backtrack and Cut is same type, we use the same type for both.
 *  The index in std::variant is used to distinguish them.
 *  
 *  We put the second template parameter as Incomplete, so that we can use different type for Incomplete.
 */
template <typename Error, typename Incomplete = size_t>
class err_mode
{

    template <size_t N, typename... Args>
    constexpr err_mode(std::in_place_index_t<N>, Args&&... args) : value(std::in_place_index<N>, std::forward<Args>(args)...) { }

public:

    using incomplete_type = Incomplete;
    using backtrack_type = Error;
    using cut_type = Error;

    constexpr err_mode(const err_mode&) = default;
    constexpr err_mode(err_mode&&) = default;

    static constexpr err_mode make_incomplete(Incomplete needed) 
    { 
        return err_mode(std::in_place_index<0>, needed); 
    }

    template <typename... Args>
    static constexpr err_mode make_backtrack(Args&&... args) 
    { 
        return err_mode(std::in_place_index<1>, std::forward<Args>(args)...); 
    }
    
    template <typename... Args>
    static constexpr err_mode make_cut(Args&&... args) 
    { 
        return err_mode(std::in_place_index<2>, std::forward<Args>(args)...); 
    }

    constexpr err_mode to_cut() const
    {
        return is_backtrack() ? make_cut(as_backtrack()) : *this;
    }

    // We use index instead of std::holds_alternative to avoid the overhead of typeid.
    constexpr bool is_incomplete() const { return value.index() == 0; }
    constexpr bool is_backtrack() const { return value.index() == 1; }
    constexpr bool is_cut() const { return value.index() == 2; }

    constexpr auto& as_incomplete() const { return std::get<0>(value); }
    constexpr auto& as_incomplete() { return std::get<0>(value); }
    
    constexpr auto& as_backtrack() { return std::get<1>(value); }
    constexpr auto& as_backtrack() const { return std::get<1>(value); }

    constexpr auto& as_cut() { return std::get<2>(value); }
    constexpr auto& as_cut() const { return std::get<2>(value); }

private:

    std::variant<Incomplete, Error, Error> value;

};



} // namespace winnow

template <typename E, typename I>
struct std::formatter<winnow::err_mode<E, I>> 
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const winnow::err_mode<E, I>& err, FormatContext& ctx) const
    {
        if (err.is_incomplete())
        {
            return std::format_to(ctx.out(), "Incomplete({})", err.as_incomplete());
        }
        else if (err.is_backtrack())
        {
            return std::format_to(ctx.out(), "Backtrack({})", err.as_backtrack());
        }
        else if (err.is_cut())
        {
            return std::format_to(ctx.out(), "Cut({})", err.as_cut());
        }
        else
        {
            return std::format_to(ctx.out(), "Unknown error");
        }
    }
};

