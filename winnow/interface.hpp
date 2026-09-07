#pragma once

namespace winnow::detail
{

template <typename Parser, typename F> struct map_parser;

template <typename Parser, typename P> struct verify_parser;

template <typename Parser, typename Value> struct value_parser;

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
};

// Runs the embedded parser repeatedly, filling the given slice with results.
// This parser fails if the input runs out before the given slice is full.
// Since we use an output iterator, the output container must be pre-sized.
// So we cannot check whether the given slice is full or not, we add
// another param `sentinel`, which used for indicating whether the
// parse should be stopped.
template <typename Parser, typename Iterator, typename Sentinel>
struct fill_parser
{
    Parser parser;
    [[no_unique_address]] Iterator iter;
    [[no_unique_address]] Sentinel sent;

    constexpr fill_parser(Parser p, Iterator i, Sentinel s) 
        : parser(std::move(p)), iter(std::move(i)), sent(std::move(s)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) /* non-const */
    {
        using E = typename Stream::error_type;
        using R = modal_result<Iterator, E>;  // Just return end 

        // Repeats the embedded parser, filling the given slice with results.
        // This parser fails if the input runs out before the given slice is full.
        for (; iter != sent; )
        {
            auto item_result = parser(stream);

            if (!item_result)
            {
                return R(std::unexpect, std::move(item_result.error()));
            }

            *iter++ = std::move(item_result.value());
        }

        return R(std::in_place, std::move(iter));
    }
};


}  // namespace winnow::detail

