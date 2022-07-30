#pragma once

#include <lv_cpp/collections/internal/common.hpp>

#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>
#include <compare>
#include <iterator>
#include <assert.h>


namespace leviathan::collections
{
	/*
		Compare:
			bool binary(arg1, arg2): arg1: key of item, arg2: value
		A container which is faster than std::set/std::map

	*/

    // default allocator is enough
    template <typename T, typename Compare, typename KeyOfValue, bool UniqueKey, std::size_t TrunkSize = 1000>
    class sorted_list
    {
		static_assert(UniqueKey, "MultiSet and MultiMap is not support now");
		using inner_container = std::vector<T>;
		using outer_container = std::vector<inner_container>;

        constexpr static bool IsTransparent = leviathan::collections::detail::is_transparent<Compare>;
        
        using Key = typename KeyOfValue::template type<T>;

        template <typename U> using key_arg_t = leviathan::collections::detail::key_arg<IsTransparent, U, Key>;

		template <bool Const>
		struct sorted_list_iterator
		{

			using in_iter = typename inner_container::iterator;
			using out_iter = typename outer_container::iterator;

			using link_type = std::conditional_t<Const, const sorted_list*, sorted_list*>;
            using value_type = std::conditional_t<Const, const T, T>;
			
            using reference = std::conditional_t<std::is_same_v<Key, T>, const value_type&, value_type&>;

			using difference_type = typename std::iterator_traits<in_iter>::difference_type;
			using iterator_category = std::bidirectional_iterator_tag;


			link_type m_c;
			std::size_t m_out_idx;
			std::size_t m_in_idx;

			constexpr sorted_list_iterator() noexcept = default;
			constexpr sorted_list_iterator(const sorted_list_iterator&) noexcept = default;

			constexpr sorted_list_iterator(const sorted_list_iterator<!Const>& rhs) noexcept requires (Const)
				: m_c{ rhs.m_c }, m_out_idx{ rhs.m_out_idx }, m_in_idx{ rhs.m_in_idx } { }

			constexpr sorted_list_iterator(link_type c, std::size_t out_idx, std::size_t in_idx = 0)
				: m_c{ c }, m_out_idx{ out_idx }, m_in_idx{ in_idx } { }

			constexpr bool operator==(const sorted_list_iterator& rhs) const noexcept = default;

			constexpr reference operator*() const noexcept 
			{ return m_c->m_lists[m_out_idx][m_in_idx]; }

			constexpr auto operator->() const noexcept 
			{ return &(this->operator*()); }

			constexpr sorted_list_iterator& operator++() noexcept
			{
				m_in_idx++;
				if (m_in_idx == m_c->m_lists[m_out_idx].size())
					m_out_idx++, m_in_idx = 0;
				return *this;
			}

			constexpr sorted_list_iterator operator++(int) noexcept
			{
				auto old = *this;
				++ *this;
				return old;
			}

			constexpr sorted_list_iterator& operator--() noexcept
			{
				if (m_in_idx == 0) 
					m_in_idx = m_c->m_lists[--m_out_idx].size();
				m_in_idx --;
				return *this;
			}

			constexpr sorted_list_iterator operator--(int) noexcept
			{
				auto old = *this;
				-- *this;
				return old;
			}

		};


    public:
        using size_type = std::size_t;

        using iterator = sorted_list_iterator<false>;
        using const_iterator = sorted_list_iterator<true>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        using key_type = Key;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using key_compare = Compare;
        using value_compare = Compare;
        using reference = value_type&;
        using const_reference = const value_type&;
        using allocator_type = std::allocator<T>;
        using pointer = std::allocator_traits<allocator_type>::pointer;
        using const_pointer = std::allocator_traits<allocator_type>::const_pointer;
        // using insert_return_type = 

    private:
        outer_container m_lists;
        size_type m_size;
        [[no_unique_address]] Compare m_cmp;

    };

};
