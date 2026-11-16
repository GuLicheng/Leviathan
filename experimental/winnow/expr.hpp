#pragma once

#include "error.hpp"
#include "utils.hpp"

namespace winnow::detail
{


template <typename I, typename O, typename E, typename Operand, typename Prefix, typename Postfix, typename Inifx>
struct expr
{
    int precedence_level;
    [[no_unique_address]] Operand parse_operand;
    [[no_unique_address]] Prefix parse_prefix;
    [[no_unique_address]] Postfix parse_postfix;
    [[no_unique_address]] Inifx parse_infix;


    template <typename Self, typename NewParsePrefix>
    constexpr expr<I, O, E, Operand, NewParsePrefix, Postfix, Inifx> 
    prefix(this Self&& self, NewParsePrefix new_parse_prefix)
    {
        return expr<I, O, E, Operand, NewParsePrefix, Postfix, Inifx>{
            .precedence_level = ((Self&&)self).precedence_level,
            .parse_operand = ((Self&&)self).parse_operand,
            .parse_prefix = std::move(new_parse_prefix),
            .parse_postfix = ((Self&&)self).parse_postfix,
            .parse_infix = ((Self&&)self).parse_infix
        };
    }

    template <typename Self, typename NewParsePostfix>
    constexpr expr<I, O, E, Operand, Prefix, NewParsePostfix, Inifx> 
    postfix(this Self&& self, NewParsePostfix new_parse_postfix)
    {
        return expr<I, O, E, Operand, Prefix, NewParsePostfix, Inifx>{
            .precedence_level = ((Self&&)self).precedence_level,
            .parse_operand = ((Self&&)self).parse_operand,
            .parse_prefix = ((Self&&)self).parse_prefix,
            .parse_postfix = std::move(new_parse_postfix),
            .parse_infix = ((Self&&)self).parse_infix
        };
    }

    template <typename Self, typename NewParseInfix>
    constexpr expr<I, O, E, Operand, Prefix, Postfix, NewParseInfix> 
    infix(this Self&& self, NewParseInfix new_parse_infix)
    {
        return expr<I, O, E, Operand, Prefix, Postfix, NewParseInfix>{
            .precedence_level = ((Self&&)self).precedence_level,
            .parse_operand = ((Self&&)self).parse_operand,
            .parse_prefix = ((Self&&)self).parse_prefix,
            .parse_postfix = ((Self&&)self).parse_postfix,
            .parse_infix = std::move(new_parse_infix)
        };
    }
};









}  // namespace winnow::detail







