#ifndef __ZIP_HPP__
#define __ZIP_HPP__

#include <concepts>
#include <utility>
#include <tuple>
#include <lv_cpp/type_list.hpp>
#include <ranges>

#include <iostream>

namespace leviathan
{

namespace ranges
{

// both of tuple size and tuple element is undefined
// it must be implemented by specializing

template <typename T>
struct tuple_size : std::tuple_size<T> { };

template <size_t N, typename T>
struct tuple_element : std::tuple_element<N, T> { };


// zip_result for zip
template <typename... Ts>
class zip_result : private std::tuple<Ts...> 
{
    using base = std::tuple<Ts...>;
public:

    using base::tuple;
    using base::operator=;
    using base::swap;

    friend constexpr auto 
    operator<=>(const zip_result&, const zip_result&) 
    noexcept(noexcept(std::declval<base>() <=> std::declval<base>())) = default;


    template <size_t N> requires (N < sizeof...(Ts))
    constexpr decltype(auto) get() noexcept
    {
        return std::get<N>(static_cast<base&>(*this));
    }

    template <size_t N> requires (N < sizeof...(Ts))
    constexpr decltype(auto) get() const noexcept
    {
        return std::get<N>(static_cast<const base&>(*this));
    }

    void show() const
    {   
        // assert Ts = (int, double, bool)
        constexpr auto res1 = noexcept(std::declval<base>() <=> std::declval<base>());
        std::cout << (*this).get<0>() << std::endl;
        std::cout << (*this).get<1>() << std::endl;
        std::cout << res1 << std::endl;
    }

};  // namespace ranges


// specialize for tuple_size
template <typename... Ts>
struct tuple_size<zip_result<Ts...>> 
    : ::leviathan::meta::size<Ts...> { };

template <size_t N, typename... Ts>
struct tuple_element<N, zip_result<Ts...>>
{
private:
    using _T = typename ::leviathan::meta::index_of<N, Ts...>::type;
public:
    using type = _T;
};

template <size_t N, typename... Ts>
struct tuple_element<N, const zip_result<Ts...>>
{
    using type = const typename tuple_element<N, zip_result<Ts...>>::type;
};



} // namespace ranges

}  // namespace leviathan

#endif

