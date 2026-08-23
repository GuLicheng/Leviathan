#pragma once

#include "error.hpp"

namespace nom
{

template <typename T, typename E>
class [[=cpp::derive::debug]] result
{
    struct [[=cpp::derive::debug]] ok { T value; };
    struct [[=cpp::derive::debug]] err { E value; };

    constexpr result(ok o) : value(std::in_place_index<0>, std::move(o)) { }
    constexpr result(err e) : value(std::in_place_index<1>, std::move(e)) { }

public:

    constexpr result(const result&) = default;
    constexpr result(result&&) = default;

    static constexpr result make_ok(T t) { return result(ok{ .value = std::move(t) }); }
    static constexpr result make_err(E e) { return result(err{ .value = std::move(e) }); }

    constexpr bool is_ok() const { return std::holds_alternative<ok>(value); }
    constexpr bool is_err() const { return std::holds_alternative<err>(value); }

    template <typename Self>
    constexpr auto&& unwrap_ok(this Self&& self) 
    { 
        return std::forward_like<Self>(std::get<ok>(self.value).value);
    }

    template <typename Self>
    constexpr auto&& unwrap_err(this Self&& self)
    { 
        return std::forward_like<Self>(std::get<err>(self.value).value);
    }

    template <typename Self, typename F>
    constexpr auto map_err(this Self&& self, F&& f)
    {
        using Recoverable = std::invoke_result_t<F, typename E::recoverable>;
        using Unrecoverable = std::invoke_result_t<F, typename E::unrecoverable>;
        
        using Err = error<Recoverable, Unrecoverable>;
        using R = result<T, Err>;

        if (self.is_ok())
        {
            return R::make_ok(std::forward_like<Self>(self).unwrap_ok());
        }
        
        auto&& e = self.unwrap_err();

        if (e.is_incomplete())
        {
            return R::make_err(Err::make_incomplete(e.as_incomplete().value));
        }
        else if (e.is_recoverable())
        {
            return R::make_err(Err::make_recoverable(std::invoke(std::forward<F>(f), std::forward_like<Self>(e).as_recoverable().value)));
        }
        else
        {
            return R::make_err(Err::make_unrecoverable(std::invoke(std::forward<F>(f), std::forward_like<Self>(e).as_unrecoverable().value)));
        }
    }

private:

    std::variant<ok, err> value;

};


template <typename I, typename O, typename E>
using iresult = result<std::pair<I, O>, error<E>>;

}  // namespace nom
