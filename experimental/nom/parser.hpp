/* 
    We only implement a very small subset of nom with nom::complete part.
    The streaming part is not implemented.
*/

#pragma once

#include "error.hpp"

namespace nom
{

template <typename ParseContext>
struct Parser 
{
    // using Output = ...
    // using Error = ... requires ParserError<ParseContext>

    // template <typename Self, typename ParseContext>
    // auto parse(this Self&& self, ParseContext& ctx) const
    // {
    //     return std::invoke(std::forward<Self>(self), ctx);
    // }
};

struct None { };


    

}  // namespace nom
