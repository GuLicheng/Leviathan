#pragma once

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
        return std::forward_like<Self>(self).value.transform_error(std::forward<F>(f));
    }

private:

    std::expected<T, E> value;

};

template <typename I, typename O, typename E>
using iresult = result<std::pair<I, O>, error<E>>;

}  // namespace nom
