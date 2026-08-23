#pragma once

#include <leviathan/extc++/expected.hpp>

#include "error.hpp"

namespace nom
{

template <typename T, typename E>
class [[=cpp::derive::debug]] result 
{

    template <typename... Args>
    constexpr result(Args&&... args) : value(std::forward<Args>(args)...) { }

public:

    using value_type = T;
    using error_type = E;

    constexpr result() = default;
    constexpr result(const result&) = default;
    constexpr result(result&&) = default;

    template <typename... Args>
    static constexpr result make_ok(Args&&... args) { return result(std::in_place, std::forward<Args>(args)...); }

    template <typename... Args>
    static constexpr result make_err(Args&&... args) { return result(std::unexpect, std::forward<Args>(args)...); }

    constexpr bool is_ok() const { return value.has_value(); }
    constexpr bool is_err() const { return !value.has_value(); }

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

// https://docs.rs/nom/latest/nom/type.IResult.html
// pub type IResult<I, O, E = Error<I>> = Result<(I, O), Err<E>>;
template <typename I, typename O, typename E = error<I>>
using iresult = result<std::pair<I, O>, err<E>>;

}  // namespace nom

