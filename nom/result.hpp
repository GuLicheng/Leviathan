#pragma once

#include <leviathan/extc++/expected.hpp>

#include "error.hpp"

namespace nom
{

// Is std::expected<T, E> compatible with rust::result<T, E>?
template <typename T, typename E>
class [[=cpp::derive::debug]] result 
{

    template <typename... Args>
    constexpr result(Args&&... args) : value(std::forward<Args>(args)...) { }

public:

    constexpr result(const result&) = default;
    constexpr result(result&&) = default;

    static constexpr result make_ok(T t) { return result(std::move(t)); }
    static constexpr result make_err(E e) { return result(std::unexpect, std::move(e)); }

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

template <typename I, typename O, typename E>
using iresult = result<std::pair<I, O>, error<E>>;

}  // namespace nom

