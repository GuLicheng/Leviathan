#pragma once

#include <leviathan/config_parser/core/parser_interface.hpp>
#include <leviathan/config_parser/core/utils.hpp>
#include <leviathan/config_parser/core/error.hpp>
#include <utility>
#include <functional>

namespace cpp::config::parser::detail
{

template <typename Parser, typename F> 
class map_parser : public parser_interface
{
    [[no_unique_address]] Parser m_parser;
    [[no_unique_address]] F m_func;

public:

    static constexpr bool is_always_succeed = false;

    template <typename Parser2, typename F2>
    constexpr map_parser(Parser2&& p, F2&& f)
        : m_parser((Parser2&&) p), m_func((F2&&) f)
    { }

    template <typename Context>
    constexpr auto operator()(Context& ctx) const
    {
        using E = typename Context::error_type;
        using R1 = std::invoke_result_t<Parser, Context&>;
        using O1 = typename R1::value_type;
        using O2 = std::invoke_result_t<F, O1>;
        // We decay the output type to handle cases where the 
        // function returns a reference or a non-decayed type.
        // Rust adopts move semantics together with value semantics,
        // and our design aligns with that.
        using R = parse_result<std::decay_t<O2>, E>;

        auto result = m_parser(ctx);

        if (!result)
        {
            return R(std::unexpect, std::move(result.error()));
        }
        return R(std::in_place, std::invoke(m_func, std::move(result.value())));
    }
};

template <typename Parser, typename F>
map_parser(Parser&&, F&&) -> map_parser<std::decay_t<Parser>, std::decay_t<F>>;

template <typename Pred>
class take_while_parser : public parser_interface
{
    [[no_unique_address]] Pred m_pred;
    occurrences<size_t> m_range;

public:

    static constexpr bool is_always_succeed = true;

    template <typename Pred2>
    constexpr take_while_parser(Pred2&& p, occurrences<size_t> r)
        : m_pred((Pred2&&) p), m_range(r) { }
    
    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using O = std::basic_string_view<typename Stream::value_type>;
        using R = parse_result<O, E>;
        
        // User should ensure that the upper is not less than lower.
        size_t count = 0;

        while (count < stream.size() && std::invoke(m_pred, stream[count]))
        {
            if (m_range.is_upper_bound(count))
            {
                break;
            }
            ++count;
        }

        if (m_range.is_less_than_lower(count))
        {
            return make_recoverable_from_input<R>(stream);
        }
        else
        {
            auto [left, right] = stream.split_at(count);
            stream = std::move(right);
            return R(std::in_place, std::move(left));
        }
    }
};

template <typename Pred>
take_while_parser(Pred&&, occurrences<size_t>) -> take_while_parser<std::decay_t<Pred>>;

template <typename... Parsers>
class sequence_parser : public parser_interface
{
    std::tuple<Parsers...> m_parsers;

    static constexpr auto indices = std::make_index_sequence<sizeof...(Parsers)>{};

public:

    static constexpr bool is_always_succeed = false;

    template <typename... Parsers2>
    constexpr sequence_parser(Parsers2&&... ps) : m_parsers((Parsers2&&) ps...) { } 

    template <typename Context>
    constexpr auto operator()(Context& ctx) const
    {
        using E = typename Context::error_type;
        using OptOs = std::tuple<std::optional<typename std::invoke_result_t<Parsers, Context&>::value_type>...>;
        using Os = std::tuple<typename std::invoke_result_t<Parsers, Context&>::value_type...>;
        using R = parse_result<Os, E>;

        OptOs opt_results;
        std::optional<err_mode<E>> err;
        
        template for (constexpr auto idx : indices)
        {
            auto& parser = std::get<idx>(m_parsers);
            auto result = parser(ctx);

            if (!result.has_value())
            {
                err.emplace(std::move(result.error()));
                break;
            }

            std::get<idx>(opt_results).emplace(std::move(result.value()));
        }

        if (err.has_value())
        {
            return make_recoverable_from_input<R>(ctx);
        }

        constexpr auto [...idx] = indices; 

        return R(std::in_place, Os(std::move(std::get<idx>(opt_results).value())...));
    }
};

template <typename... Parsers>
sequence_parser(Parsers&&... ps) -> sequence_parser<std::decay_t<Parsers>...>;

template <typename CharT>
class literal_parser : public parser_interface
{
    using literal_type = std::basic_string_view<CharT>;

    literal_type m_constant;

public:

    static constexpr bool is_always_succeed = false;

    constexpr literal_parser(literal_type t) : m_constant(t) { }

    template <typename Context>
    constexpr auto operator()(Context& ctx) const
    {
        // Rust winnow return a part of input/stream. 
        // We just return slices of the input stream.
        using E = typename Context::error_type;
        using O = literal_type;
        using R = parse_result<literal_type, E>;

        if (ctx.match(m_constant, false))
        {
            auto [left, right] = ctx.split_at(m_constant.size());
            ctx = std::move(right);
            return R(std::in_place, std::move(left));
        }
        else
        {
            return make_recoverable_from_input<R>(ctx);
        }
    }
};



}  // namespace cpp::config::parser::detail


