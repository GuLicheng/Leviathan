#pragma once

#include <cstddef>
#include <memory>

namespace leviathan::collections
{
	// for vector/list/deque/set/unordered_set
	enum class config_type
	{
		set,
		map
	};

	template <typename K, typename Allocator>
	struct identity
	{
		using key_type = K;
		using value_type = K;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		using allocator_type = Allocator;

		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = typename std::allocator_traits<allocator_type>::pointer;
		using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

		constexpr static auto config = config_type::set;

		template <typename P1, typename P2>
		constexpr static bool compare_keys(const P1 &p1, const P2 &p2) noexcept
		{
			return p1 == p2;
		}

		template <typename U>
		constexpr static const auto &get_key(const U &x) noexcept
		{
			return x;
		}
	};

	// for map/unordered_map
	template <typename K, typename V, typename Allocator>
	struct select1st
	{
		using key_type = K;
		using mapped_type = V;
		using value_type = std::pair<const key_type, mapped_type>;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		using allocator_type = Allocator;

		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = typename std::allocator_traits<allocator_type>::pointer;
		using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

		constexpr static auto config = config_type::map;

		template <typename P1, typename P2>
		constexpr static bool compare_keys(const P1 &p1, const P2 &p2) noexcept
		{
			return p1.first == p2.first;
		}

		template <typename U>
		constexpr static const auto& get_key(const U &x) noexcept
		{
			return x.first;
		}
	};

	template <typename K, typename Compare, typename Allocator>
	struct set_config : identity<K, Allocator>
	{
		using key_compare = Compare;
		using typename identity<K, Allocator>::key_type;

		struct value_compare
		{
			constexpr value_compare(key_compare c) : comp{c} {}

			[[no_unique_address]] Compare comp;

			constexpr auto operator()(const key_type &lhs, const key_type &rhs) const
				noexcept(noexcept(comp(lhs, rhs)))
			{
				return comp(lhs, rhs);
			}
		};
	};

	template <typename K, typename V, typename Compare, typename Allocator>
	struct map_config : select1st<K, V, Allocator>
	{
		using key_compare = Compare;
		using typename select1st<K, V, Allocator>::value_type;
		using typename select1st<K, V, Allocator>::key_type;

		struct value_compare
		{
			constexpr value_compare(key_compare c) : comp{c} {}

			constexpr auto operator()(const value_type &lhs, const value_type &rhs) const
				noexcept(noexcept(comp(lhs.first, rhs.first)))
			{
				return comp(lhs.first, rhs.first);
			}

			[[no_unique_address]] key_compare comp;
		};
	};

	template <typename K, typename V, typename HashFunction, typename KeyEqual, typename Allocator>
	struct hash_map_config : select1st<K, V, Allocator>
	{
		using hasher = HashFunction;
		using key_equal = KeyEqual;
	};

	template <typename T, typename HashFunction, typename KeyEqual, typename Allocator>
	struct hash_set_config : identity<T, Allocator>
	{
		using hasher = HashFunction;
		using key_equal = KeyEqual;
	};

}
