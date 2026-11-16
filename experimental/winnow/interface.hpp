/*
    https://docs.rs/winnow/latest/winnow/trait.Parser.html#provided-methods

    - void
    - map
    - value
    - cut
    - and_then
    - context
    - verify


    - parse : operator()
    - [x] parse_iter
    - [x] parse_peek
    - [x] by_ref           -> std::ref/std::cref is OK
    - [x] default_value    -> <T>.map([](auto&&) { return T(); })
    - [x] output_into
    - [x] take
    - [x] with_take
    - [x] span
    - [x] span
    - [x] with_span
    - [x] try_map
    - [x] verify_map
    - [x] flat_map
    - [x] parse_to         -> .map(std::str::FromStr)
    - [x] context_with
    - [x] map_err
    - [x] complete_err
    - [x] err_into
    - [x] retry_after
    - [x] resume_after

*/

#pragma once

#include "utils.hpp"

namespace winnow::detail
{

template <typename Parser, typename F> struct map_parser;

template <typename Parser, typename P> struct verify_parser;

template <typename Parser, typename Value> struct value_parser;

template <typename Parser> struct cut_err_parser;

template <typename Parser, typename Context> struct context_parser;

template <typename Parser, typename AndThenParser> struct and_then_parser;

struct parser_interface
{
    template <typename Self, typename F>
    constexpr auto map(this Self&& self, F&& f)
    {
        return map_parser<std::decay_t<Self>, std::decay_t<F>>{(Self&&)self, (F&&)f};
    }

    template <typename Self, typename P>
    constexpr auto verify(this Self&& self, P&& p)
    {
        return verify_parser<std::decay_t<Self>, std::decay_t<P>>{(Self&&)self, (P&&)p};
    }

    template <typename Self, typename Value>
    constexpr auto value(this Self&& self, Value&& value)
    {
        return value_parser<std::decay_t<Self>, std::decay_t<Value>>{(Self&&)self, (Value&&)value};
    }

    template <typename Self, typename F>
    constexpr auto void_(this Self&& self, F&& f)
    {
        return ((Self&&)self).value(unit());
    }

    template <typename Self>
    constexpr auto cut(this Self&& self)
    {
        return cut_err_parser<std::decay_t<Self>>{(Self&&)self};
    }

    template <typename Self, typename Context>
    constexpr auto context(this Self&& self, Context&& context)
    {
        return context_parser<std::decay_t<Self>, std::decay_t<Context>>{(Self&&)self, (Context&&)context};
    }

    template <typename Self, typename AndThenParser>
    constexpr auto and_then(this Self&& self, AndThenParser&& f)
    {
        return and_then_parser<std::decay_t<Self>, std::decay_t<AndThenParser>>{(Self&&)self, (AndThenParser&&)f};
    }
};

}  // namespace winnow::detail

