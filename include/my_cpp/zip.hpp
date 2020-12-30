#ifndef __ZIP_HPP__
#define __ZIP_HPP__

#include <iostream>
#include <ranges>
#include <concepts>
#include <tuple>

#include "template_info.hpp"
#include "type_list.hpp"



namespace detail
{



}



/* helper mate */
template <typename Rng>
struct get_return
{
    using iter_t = std::ranges::iterator_t<Rng>;
    using type = decltype(*std::declval<iter_t>());
};
template <typename ... Rngs>
struct traits_iterator_dereference_type
{
    using type = std::tuple<typename get_return<Rngs>::type ...>;
};

template <typename List>
struct TransFormForTuple;

template <typename... Ts>
struct TransFormForTuple<std::tuple<Ts...>>
{
    using type = std::tuple<std::add_const_t<Ts>...>;
};

template <typename T>
struct add_const_lvalue_reference
    : std::add_lvalue_reference<std::add_const_t<std::decay_t<T>>> { };

// Rngs for inner_array[x](x > 0), std::vector, std::list, std::set or some usr-defind containers
// but not std::vector& or other reference type
template <std::ranges::range Rng1, std::ranges::range... Rngs>
class zip_iterator
{


    using self = zip_iterator;
public:


    using value_type = std::tuple< 
                std::ranges::iterator_t<Rng1>, 
                std::ranges::iterator_t<Rngs>...>;
    // std::tuple<iterator1, iterator2...>

    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;

    using iterator_category = std::common_type_t<typename std::iterator_traits<std::ranges::iterator_t<Rngs>>::iterator_category ...>;

    using element_type = typename traits_iterator_dereference_type<Rng1, Rngs...>::type;
    using reference = element_type;
    using const_reference = typename leviathan::meta::transform<add_const_lvalue_reference, element_type>::type;
    // std::tuple<int&, const int&...>
/*
    template <typename List>
    struct add_lvalue_reference;

    template <typename... Ts>
    struct add_lvalue_reference<std::tuple<Ts...>> 
        : std::enable_if<true, std::tuple<std::add_lvalue_reference_t<Ts>...>> { };
*/

    // using value_type_reference = leviathan::type::add_lvalue_reference_t<element_type>;


    constexpr zip_iterator(Rng1& rng1, Rngs&... rngs) 
        : m_data{std::ranges::begin(rng1), std::ranges::begin(rngs)...} /* , m_sentry{std::ranges::end(rng1)} */
    {
        // std::cout << "address of first element is: " << &*std::get<0>(m_data) << std::endl; 
    }

    constexpr zip_iterator(Rng1& rng1)
    {
        std::get<0>(m_data) = std::ranges::end(rng1);
    }

    void show()
    {
        // PrintTypeCategory(m_data);
        // std::cout << "show ......\n";
        std::cout << "address of first elements is :" << &*std::get<0>(m_data) << std::endl;
        // std::cout << *std::get<1>(m_data) << std::endl;
        // std::cout << *std::get<2>(m_data) << std::endl;
    }

    constexpr self& operator++() noexcept
    {
        auto get_item = []<typename... Iter>(Iter&... iter)
        {
            // std::ranges::next(iter);
            // (PrintTypeCategory(iter), ...);
            (++iter, ...);
        };
        std::apply(get_item, this->m_data);
        return *this;
    }

    constexpr self operator++(int) noexcept
    {
        auto old = *this;
        ++ *this;
        return old;   
    }

    constexpr element_type
    operator*() noexcept
    {
        constexpr auto Size = std::tuple_size_v<value_type>;
        return this->dereference_impl(
            std::make_index_sequence<Size>());
    }

    // typename TransFormForTuple<element_type>::type
    constexpr element_type
    operator*() const noexcept
    {
        constexpr auto Size = std::tuple_size_v<value_type>;
        return this->dereference_impl(
            std::make_index_sequence<Size>());
    }

    constexpr bool operator==(const self& rhs) const noexcept 
    {
        // std::cout << "sdasd" << (std::get<0>(m_data) == rhs.m_sentry) << std::endl;
        return std::get<0>(m_data) == std::get<0>(rhs.m_data);
    }


    constexpr bool operator!=(const self& rhs) const noexcept 
    {
        return !(this->operator==(rhs));
    }



private:

    value_type m_data;

    template <size_t... Idx>
    constexpr element_type
    dereference_impl(std::index_sequence<Idx...>)
    {
        // std::cout << *std::get<0>(m_data) << '-' << *std::get<1>(m_data) << '-' << *std::get<2>(m_data) << std::endl;
        return {*std::get<Idx>(m_data) ...};
    }

};


template <std::ranges::range... Ts>
constexpr auto zip_begin(Ts&&... rngs)
{
    // the lifetime of rngs must not be temperory
    return zip_iterator<std::remove_reference_t<Ts>...>(std::forward<Ts>(rngs)...);
}

template <std::ranges::range T1, std::ranges::range... Ts>
constexpr auto zip_end(T1&& rng1, Ts&&... rngs)
{
    return zip_iterator<std::remove_reference_t<T1>, 
                        std::remove_reference_t<Ts>...>
                        (std::forward<T1>(rng1));
}


#endif