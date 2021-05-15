#ifndef __CONCAT_HPP__
#define __CONCAT_HPP__

#include <lv_cpp/io/console.hpp>

#include <lv_cpp/tuples/algorithm.hpp>
#include <lv_cpp/utils/iter.hpp>
#include <concepts>
#include <iterator>
#include <ranges>

namespace leviathan::ranges
{

    template <std::ranges::input_range Rg1, std::ranges::input_range... Rgs>
    struct concat_view : public ::std::ranges::view_interface<concat_view<Rg1, Rgs...>>
    {
    private:
        static_assert(std::conjunction_v<
            std::is_same
            <
                std::iter_value_t<Rg1>, std::iter_value_t<Rgs>
            > ...>);
        constexpr static ssize_t value_size = sizeof...(Rgs) + 1;
        using _ValueType = std::iter_value_t<Rg1>;
        using _DifferenceType = std::iter_difference_t<Rg1>;
        using _ReferenceType = _ValueType&;

        using _IteratorMemberType = std::tuple<std::ranges::iterator_t<Rg1>, std::ranges::iterator_t<Rgs>...>;
        using _SentinelMemberType = std::tuple<std::ranges::sentinel_t<Rg1>, std::ranges::sentinel_t<Rgs>...>;

        struct _Sentinel 
            : leviathan::iterator_interface<_Sentinel, _ValueType, std::input_iterator_tag, _ReferenceType,_DifferenceType>
        { 
        };

        struct _Iterator 
            : leviathan::iterator_interface<_Iterator, _ValueType, std::input_iterator_tag, _ReferenceType,_DifferenceType>
        {
            _Iterator() : m_iters{}, m_sens{}
            {
            }
            _Iterator(_IteratorMemberType iters, _SentinelMemberType sens) 
                : m_iters{iters}, m_sens{sens}
            {
            }
            
            _Iterator(const _Iterator&) = default;
            _Iterator& operator=(const _Iterator&) = default;

            _IteratorMemberType m_iters;
            _SentinelMemberType m_sens;

            // dereference, equal, next
            constexpr _ReferenceType dereference() const 
            {     
                const auto idx = leviathan::tuple::tuple_mismatch(m_iters, m_sens, std::equal_to<>());
                auto& res = dereference_impl<0, value_size>(idx);         
                return res;
            }
            
            template <int Left, int Right>
            _ReferenceType dereference_impl(int idx) const
            {
                if constexpr (Left + 1 < Right)
                {
                    if (Left == idx)
                        return *std::get<Left>(m_iters);
                    else
                        return dereference_impl<Left + 1, Right>(idx);
                }
                else
                {
                    return *std::get<Left>(m_iters);
                }
            }


            constexpr _Iterator& next()
            {
                const auto idx = leviathan::tuple::tuple_mismatch(m_iters, m_sens, std::equal_to<>());
                leviathan::tuple::dynamic_set(m_iters, [](auto& iter) { ++iter; }, idx);
                return *this;
            }

            constexpr auto equal_to(const _Iterator& rhs) const noexcept
            {
                const auto idx = leviathan::tuple::tuple_mismatch(m_iters, m_sens, std::equal_to<>());
                return value_size == idx ? 0 : -1;
            }

            constexpr auto equal_to(const _Sentinel& rhs) const noexcept
            {
                const auto idx = leviathan::tuple::tuple_mismatch(m_iters, m_sens, std::equal_to<>());
                return value_size == idx ? 0 : -1;
            }

        };
    public:

        _IteratorMemberType m_begins;
        _SentinelMemberType m_ends;

        concat_view () = default;
        concat_view(Rg1& rg1, Rgs&... rgs) 
            : m_begins{std::ranges::begin(rg1), std::ranges::begin(rgs)...}, m_ends{std::ranges::end(rg1), std::ranges::end(rgs)...}
        {
        } 

        constexpr _Iterator begin()
        {
            return _Iterator(m_begins, m_ends); 
        }

        constexpr _Sentinel end()
        {
            return _Sentinel{};
        }

    }; // end of class

}


#endif