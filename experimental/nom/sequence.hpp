/*
    We only implement a very small subset of nom with nom::complete part.
    The streaming part is not implemented.
*/
#pragma once

#include "error.hpp"
#include "internal.hpp" 

namespace nom
{

// Gets an object from the first parser, then gets another object from the second parser.
inline constexpr struct 
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1 f1, F2 f2)
    {
        return And<F1, F2>(std::move(f1), std::move(f2));
    }
} pair;

// Matches an object from the first parser and discards it, 
// then gets an object from the second parser.
inline constexpr struct 
{
    template <typename First, typename Second>
    static constexpr auto operator()(First first, Second second)
    {
        return Preceded<First, Second>(std::move(first), std::move(second));
    }
} preceded;

// Gets an object from the first parser, then matches an object 
// from the sep_parser and discards it, then gets another object 
// from the second parser.
inline constexpr struct 
{
    template <typename First, typename Sep, typename Second>
    static constexpr auto operator()(First first, Sep sep, Second second)
    {
        return pair(
            std::move(first), 
            preceded(std::move(sep), std::move(second))
        );
    }
} separated_pair;

// Gets an object from the first parser, then matches 
// an object from the second parser and discards it.
inline constexpr struct 
{
    template <typename First, typename Second>
    static constexpr auto operator()(First first, Second second)
    {
        return Terminated<First, Second>(std::move(first), std::move(second));
    }
} terminated;

// Matches an object from the first parser and discards it, 
// then gets an object from the second parser, and finally matches 
// an object from the third parser and discards it.
inline constexpr struct 
{
    template <typename First, typename Second, typename Third>
    static constexpr auto operator()(First first, Second second, Third third)
    {
        return preceded(std::move(first), terminated(std::move(second), std::move(third)));
    }
} delimited;

} // namespace nom

