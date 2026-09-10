#pragma once

namespace winnow::detail
{

template <typename Parser, typename F> struct map_parser;

template <typename Parser, typename P> struct verify_parser;

template <typename Parser, typename Value> struct value_parser;

template <typename Parser> struct cut_err_parser;

template <typename Parser, typename Context> struct context_parser;

struct parser_interface
{
    template <typename Self, typename F>
    constexpr auto map(this Self&& self, F&& f)
    {
        return map_parser<std::decay_t<Self>, F>{(Self&&)self, (F&&)f};
    }

    template <typename Self, typename P>
    constexpr auto verify(this Self&& self, P&& p)
    {
        return verify_parser<std::decay_t<Self>, P>{(Self&&)self, (P&&)p};
    }

    template <typename Self, typename Value>
    constexpr auto value(this Self&& self, Value&& value)
    {
        return value_parser<std::decay_t<Self>, Value>{(Self&&)self, (Value&&)value};
    }

    template <typename Self>
    constexpr auto cut(this Self&& self)
    {
        return cut_err_parser<std::decay_t<Self>>{(Self&&)self};
    }

    template <typename Self, typename Context>
    constexpr auto context(this Self&& self, Context&& context)
    {
        return context_parser<std::decay_t<Self>, Context>{(Self&&)self, (Context&&)context};
    }
};

}  // namespace winnow::detail

