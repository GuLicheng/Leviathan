#pragma once

#include <leviathan/config_parser/core/parser_interface.hpp>
#include <leviathan/config_parser/core/utils.hpp>
#include <utility>
#include <functional>

namespace cpp::config::parser::detail
{

template <typename Parser, typename F> 
class map_parser : parser_interface
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
struct take_while_parser : parser_interface
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
        using R = modal_result<O, E>;
        
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
            // It will return an ErrMode::Backtrack(_) if the 
            // set of tokens wasn’t met or is out of occurrences range.
            return make_backtrack_from_input<R>(stream);
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

}  // namespace cpp::config::parser::detail


