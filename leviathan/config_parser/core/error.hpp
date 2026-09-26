#pragma once

#include <memory>  
#include <format>
#include <expected>
#include <any>

namespace cpp::config
{

template <typename Error>
class err_mode
{
public:

    using error_type = Error;
    using value_type = Error;

    template <typename... Args>
    constexpr err_mode(bool recoverable, Args&&... args)
        : m_recoverable(recoverable), m_error((Args&&)args...)
    { }

    constexpr err_mode(const err_mode& other) = default;
    constexpr err_mode(err_mode&& other) = default;
    constexpr err_mode& operator=(const err_mode& other) = default;
    constexpr err_mode& operator=(err_mode&& other) = default;

    template <typename... Args>
    constexpr static err_mode make_recoverable(Args&&... args)
    {
        return err_mode(true, (Args&&)args...);
    }

    template <typename... Args>
    constexpr static err_mode make_fatal(Args&&... args)
    {
        return err_mode(false, (Args&&)args...);
    }

    constexpr bool is_recoverable() const { return m_recoverable; }
    constexpr bool is_fatal() const { return !m_recoverable; }
    
    constexpr void switch_to_recoverable() { m_recoverable = true; }
    constexpr void switch_to_fatal() { m_recoverable = false; }

    template <typename Self>
    constexpr auto&& error(this Self&& self)
    {
        return ((Self&&)self).m_error;
    }

    template <typename Self>
    constexpr auto&& operator*(this Self&& self)
    {
        return ((Self&&)self).error();
    }

    template <typename Self>
    constexpr auto& operator->(this Self& self)
    {
        return std::addressof(self.m_error);
    }

private:

    /* Whether the error is recoverable */
    bool m_recoverable;
    
    /* The error instance */
    Error m_error;
};

template <typename T, typename E>
using parse_result = std::expected<T, err_mode<E>>;

/**
 * @brief Traits for handling errors of type `E` in the configuration parser.
 * @tparam E The type of error to handle.
 * 
 *  User can specialize this trait to provide custom behavior for handling errors of type `E`.
 */
template <typename E> struct error_traits;

////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------
// winnow::error::StrContext
// ------------------------------
enum class str_context_kind
{
    label,
    expected,
    description,
};

struct str_context
{
    str_context_kind kind;
    std::string_view text;
};

// ------------------------------
// winnow::error::ContextError
// ------------------------------
struct context_error
{
    std::vector<str_context> context_stack;
    std::optional<std::any> cause;
};

template <>
struct error_traits<context_error>
{
    template <typename Stream>
    static constexpr context_error from_input(const Stream& /*stream*/, const char* message = nullptr)
    {
        return context_error {
            .context_stack = {},
            .cause = std::nullopt
        };
    }

    template <typename Stream, typename Context>
    static constexpr void add_context(context_error& e, Stream& stream, Context ctx)
    {
        e.context_stack.push_back(std::move(ctx));
    }

    // template <typename Stream, typename Ext>
    // static constexpr context_error from_external(const Stream& /*stream*/, Ext&& ext)
    // {
    //     context_error e{};
    //     e.cause = std::forward<Ext>(ext);
    //     return e;
    // }
    
};

template <typename R, typename Context>
constexpr auto make_recoverable_from_input(Context& ctx, const char* message = nullptr)
{
    // R is modal_result
    using ErrMode = typename R::error_type;
    using O = typename R::value_type;
    using E = typename ErrMode::error_type;
    return parse_result<O, E>(
        std::unexpect, ErrMode::make_recoverable(error_traits<E>::from_input(ctx, message))
    );
}


} // namespace cpp::config

template <typename E>
struct std::formatter<cpp::config::err_mode<E>> 
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const cpp::config::err_mode<E>& err, FormatContext& ctx) const
    {
        if (err.is_recoverable())
        {
            return std::format_to(ctx.out(), "Recoverable({})", *err);
        }
        else
        {
            return std::format_to(ctx.out(), "Unrecoverable({})", *err);
        }
    }
};

