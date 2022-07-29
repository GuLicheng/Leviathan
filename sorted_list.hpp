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

    template <typename T, typename Compare, typename Allocator, typename KeyOfValue, bool UniqueKey, std::size_t TrunkSize = 1000>
    class sorted_list
    {
		static_assert(UniqueKey, "MultiSet and MultiMap is not support now");
        using inner_alloc = Allocator;
		using inner_container = std::vector<T, Allocator>;
        using outer_alloc = std::allocator_traits<Allocator>::template rebind_alloc<inner_container>;
		using outer_container = std::vector<inner_container, outer_alloc>;

        constexpr static bool IsTransparent = leviathan::collections::detail::is_transparent<Compare>;
        
        using Key = typename KeyOfValue::template type<T>;

        template <typename U> using key_arg_t = leviathan::collections::detail::key_arg<IsTransparent, U, Key>;


    public:
        using size_type = std::size_t;
        using allocator_type = Allocator;

        // using iterator = tree_iterator<false>;
        // using const_iterator = tree_iterator<true>;
        // using reverse_iterator = std::reverse_iterator<iterator>;
        // using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        using key_type = Key;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using key_compare = Compare;
        using value_compare = Compare;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = std::allocator_traits<Allocator>::pointer;
        using const_pointer = std::allocator_traits<Allocator>::const_pointer;
        // using insert_return_type = 

    private:
        outer_container m_lists;
        size_type m_size;
        [[no_unique_address]] Compare m_cmp;

    };

};

#endif