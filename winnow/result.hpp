#pragma once

#include <leviathan/extc++/expected.hpp>

#include "error.hpp"

namespace winnow
{

template <typename T, typename E>
class result 
{

    template <typename... Args>
    constexpr result(Args&&... args) : value(std::forward<Args>(args)...) { }

public:

    using value_type = T;
    using error_type = E;

    // constexpr result() = default;
    constexpr result(const result&) = default;
    constexpr result(result&&) = default;
    constexpr result& operator=(const result&) = default;
    constexpr result& operator=(result&&) = default;

    template <typename... Args>
    static constexpr result make_ok(Args&&... args) { return result(std::in_place, std::forward<Args>(args)...); }

    template <typename... Args>
    static constexpr result make_err(Args&&... args) { return result(std::unexpect, std::forward<Args>(args)...); }

    constexpr bool is_ok() const { return value.has_value(); }
    constexpr bool is_err() const { return !value.has_value(); }

    constexpr operator bool() const { return is_ok(); }

    // Maybe we can derived std::expected to avoid this boilerplate code
    template <typename Self>
    constexpr auto&& unwrap_ok(this Self&& self) 
    { 
        return std::forward_like<Self>(self.value.value());
    }

    template <typename Self>
    constexpr auto&& unwrap_err(this Self&& self)
    { 
        return std::forward_like<Self>(self.value.error());
    }

private:

    std::expected<T, E> value;

};

// https://docs.rs/winnow/1.0.0/winnow/error/type.ModalResult.html
// template <typename O, typename E>
// using modal_result = result<O, err_mode<E>>;

template <typename O, typename E>
using modal_result = std::expected<O, err_mode<E>>;

template <typename R, typename Stream>
constexpr auto make_backtrack_from_input(Stream& stream, const char* message = nullptr)
{
    // R is modal_result
    using ErrMode = typename R::error_type;
    using O = typename R::value_type;
    using E = typename ErrMode::error_type;
    return modal_result<O, E>(
        std::unexpect, ErrMode::make_backtrack(error_traits<E>::from_input(stream, message))
    );
}


}  // namespace winnow

template <typename O, typename E>
struct std::formatter<winnow::modal_result<O, E>> 
{
    template <typename FormatContext>
    static constexpr auto parse(FormatContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    static constexpr auto format(const winnow::modal_result<O, E>& r, FormatContext& ctx) 
    {
        if (r.is_ok())
        {
            return std::format_to(ctx.out(), "Ok({:n})", r.value());
        }
        else
        {
            return std::format_to(ctx.out(), "Err({})", r.error());
        }
    }
};

