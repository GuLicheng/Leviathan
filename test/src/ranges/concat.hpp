#ifndef __CONCAT_HPP__
#define __CONCAT_HPP__

#include <lv_cpp/tuples/algorithm.hpp>
#include <lv_cpp/utils/iter.hpp>
#include <concepts>
#include <iterator>

namespace leviathan::ranges
{


    template <typename T1, typename... Ts> 
    strcut concat_iterator : 
        iterator_interface<concat_iterator<T1, Ts...>, std::iter_value_t<T1>, std::forward_iterator_tag>
    {
        static_assert(std::conjuction<std::forward_iterator<T1>, std::forward_iterator<Ts>...>);
        static_assert(std::conjuction<std::same_as<std::iter_value_t<T1>, std:iter_value_t<Ts>>...>);

        
        concat_iterator() : m_iter{}
        {
        }

        concat_iterator(T1 t1, Ts... ts) : {std::move(t1), std::move(ts)...} 
        {
        }

        concat_iterator(const concat_iterator&) = default;
        concat_iterator(concat_iterator&&) = default;
        concat_iterator& operator=(const concat_iterator&) = default;
        concat_iterator& operator=(concat_iterator&&) = default;

        std::tuple<T1, Ts...> m_iter;

        using base = iterator_interface<concat_iterator<T1, Ts...>, std::iter_value_t<T1>, std::forward_iterator_tag>;
        using base::value_type;
        using base::reference_type;
        using base::difference_type;

        reference_type dereference() const;

        bool equal(const concat_iterator& rhs) const;



    };




}

#endif