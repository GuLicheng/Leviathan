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

    template <typename Self, typename F>
    constexpr auto map_err(this Self&& self, F&& f)
    {
        using E2 = std::invoke_result_t<F, E>;
        using R = result<T, E2>;

        if (self.is_ok())
        {
            return R::make_ok(std::forward_like<Self>(self).unwrap_ok());
        }
        else
        {
            return R::make_err(std::invoke(std::forward<F>(f), std::forward_like<Self>(self).unwrap_err()));
        }
    }

private:

    std::expected<T, E> value;

};

// https://docs.rs/winnow/1.0.0/winnow/error/type.ModalResult.html
template <typename O, typename E>
using modal_result = result<O, err_mode<E>>;

template <typename E1, typename E2> 
struct replace_error;

template <typename O, typename E1, typename E2>
struct replace_error<modal_result<O, E1>, E2>
{
    using type = modal_result<O, E2>;
};

template <typename O1, typename O2>
struct replace_value;

template <typename O1, typename O2, typename E>
struct replace_value<modal_result<O1, E>, O2>
{
    using type = modal_result<O2, E>;
};

template <typename Result>
struct inner_error;

template <typename O, typename E>
struct inner_error<modal_result<O, E>>
{
    using type = E;
};

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
            return std::format_to(ctx.out(), "Ok({:n})", r.unwrap_ok());
        }
        else
        {
            return std::format_to(ctx.out(), "Err({})", r.unwrap_err());
        }
    }
};

