#pragma once

#include <lv_cpp/collections/internal/common.hpp>
#include <lv_cpp/iterators/norm_iterator.hpp>

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

		sorted_list() noexcept(std::is_nothrow_default_constructible_v<Compare>) 
			: m_lists{ }, m_cmp{ }, m_size { } { }

		explicit sorted_list(const Compare& cmp) : m_lists{ }, m_cmp { cmp }, m_size { 0 } { }

		sorted_list(const sorted_list&) = default;
		sorted_list(sorted_list&&) 
			noexcept(std::is_nothrow_move_constructible_v<Compare> && std::is_nothrow_move_constructible_v<outer_container>) = default;

		sorted_list& operator=(const sorted_list&) = default;
		sorted_list& operator=(sorted_list&&) 
			noexcept(std::is_nothrow_assignable_v<Compare> && std::is_nothrow_move_assignable_v<outer_container>) = default;

		size_type size() const  
		{ return m_size; }
		
		bool empty() const  
		{ return m_size == 0; }

		// simple API for iterator
		iterator begin()  
		{ return { this, 0 }; }
		
		iterator end()  
		{ return { this, m_lists.size() }; }

		const_iterator begin() const  
		{ return const_cast<sorted_list&>(*this).begin(); }

		const_iterator end() const  
		{ return const_cast<sorted_list&>(*this).end(); }

		const_iterator cbegin() const  
		{ return begin(); }

		const_iterator cend() const  
		{ return end(); }

		reverse_iterator rbegin()  
		{ return std::make_reverse_iterator(end()); } 

		reverse_iterator rend()  
		{ return std::make_reverse_iterator(begin()); } 

		const_reverse_iterator rbegin() const  
		{ return std::make_reverse_iterator(end()); } 

		const_reverse_iterator rend() const  
		{ return std::make_reverse_iterator(begin()); } 
		
		const_reverse_iterator rcbegin() const  
		{ return rbegin(); } 

		const_reverse_iterator rcend() const  
		{ return rend(); } 

        // Modifiers
		std::pair<iterator, bool> insert(const value_type& x)
		{ return insert_impl(x); }

		std::pair<iterator, bool> insert(value_type&& x)
		{ return insert_impl(std::move(x)); }
        
		size_type erase(const key_type& x)
		{
			auto old_size = size();
			auto iter = find(x);
			if (x != end())
				remove_impl(iter);
			return old_size - size();
		}

        // Lookup
        template <typename K = key_type>
        iterator lower_bound(const key_arg_t<K>& x)
		{
			auto [i, j] = find_item_by_key(x);
			return i == m_lists.size() ? end() : iterator(this, i, j);
		}

        template <typename K = key_type>
		iterator find(const key_arg_t<K>& x)
		{
			auto lower = lower_bound(x);
			return lower == end() || m_cmp(x, KeyOfValue()(*lower)) ? end() : lower;
		}

		size_type bucket_count() const 
		{ return m_lists.size(); }


    private:
        outer_container m_lists;
        [[no_unique_address]] Compare m_cmp;
        size_type m_size;

		template <typename U> 
		std::pair<iterator, bool> insert_impl(U&& val) 
		{
			bool succeed;
			std::size_t out_idx, in_idx;
			if (m_lists.size())
			{
				auto [i, j] = find_item_by_key(KeyOfValue()(val));
				if (i == m_lists.size())
				{
					// insert last position
					m_lists.back().emplace_back((U&&) val);
					i = m_lists.size() - 1;
					j = m_lists.back().size() - 1;
					succeed = true;
				}
				else
				{
					if (!m_cmp(KeyOfValue()(val), KeyOfValue()(m_lists[i][j])))
					{
						succeed = false;
					}
					else
					{
						auto& v = m_lists[i];
						auto ret_iter = v.insert(v.begin() + j, (U&&)val);
						j = std::ranges::distance(v.begin(), ret_iter);
						succeed = true;
					}
				}

				bool expanded = expand(i);
				if (expanded && j > TrunkSize)
				{
					i++;
					j -= TrunkSize;
				}
				out_idx = i, in_idx = j;
			}
			else
			{
				m_lists.emplace_back(inner_container{ (U&&) val });
				out_idx = in_idx = 0;
				succeed = true;
			}
			m_size += succeed;
			return { iterator(this, out_idx, in_idx), succeed };
		}

		// return lower_bound item
		template <typename K>
		std::pair<std::size_t, std::size_t> find_item_by_key(const K& k) const 
		{
			auto bucket_loc = std::lower_bound(m_lists.begin(), m_lists.end(), k, [this](const auto& vec, const auto& value) { 
				return m_cmp(KeyOfValue()(vec.back()), value);
			});
			
			if (bucket_loc == m_lists.end())
				return { m_lists.size(), 0 };
			auto iter = std::lower_bound(bucket_loc->begin(), bucket_loc->end(), k, [this](const auto& item, const auto& value) { 
				return m_cmp(KeyOfValue()(item), value);
			});
			return { std::ranges::distance(m_lists.begin(), bucket_loc), std::ranges::distance(bucket_loc->begin(), iter) };
		}


		iterator remove_impl(const_iterator pos) 
		{
			auto i = pos.m_out_idx, j = pos.m_in_idx;
			m_lists[i].erase(m_lists[i].begin() + j);
			--m_size;

			shrink(i, j);	
			// if (m_lists[i].empty())
			// {
			// 	m_lists.erase(m_lists.begin() + i);
			// 	return { this, i, j };
			// }

			// if (j == m_lists[i].size())
				// return { this, i + 1, 0 };

			return { this, i, j };
		}

		// move elements more than truck_size in bucket[pos] into next bucket
		bool expand(size_t pos)
		{
			if (m_lists[pos].size() > TrunkSize * 2)
			{
				auto& bucket = m_lists[pos];
				std::vector<T> half;
				// move
				half.reserve(std::ranges::distance(bucket.begin() + TrunkSize, bucket.end()));
				if constexpr (std::is_nothrow_constructible_v<T>)
				{
					half.insert(half.end(),
						std::make_move_iterator(bucket.begin() + TrunkSize),
						std::make_move_iterator(bucket.end()));
				}
				else
				{
					half.insert(half.end(), bucket.begin() + TrunkSize, bucket.end());
				}

				// half.insert(half.end(), 
				// 	make_move_if_noexcept_iterator(bucket.begin() + TrunkSize), 
					// make_move_if_noexcept_iterator(bucket.end()));

				bucket.erase(bucket.begin() + TrunkSize, bucket.end());
				m_lists.insert(m_lists.begin() + pos + 1, std::move(half));
				return true;
			}
			return false;
		}

		// Make sure each bucket will not be empty !
		void shrink(size_t& out_idx, size_t& in_idx)
		{
			// We merge elements in prev and out_idx into prev firstly,
			// and then try to split prev and out_idx
			if (m_lists[out_idx].size() < TrunkSize / 2 && m_lists.size() > 1)
			{
				if (out_idx == 0)
					out_idx++;
				auto prev = out_idx - 1;
				// move elements of out_idx into prev and check whether the number of elements in prev is 
				// greater than TrunkSize * 2. If prev.size() > TruckSize * 2, we move half of elements
				// from prev to out_idx, otherwise erase out_idx. 
				in_idx += m_lists.size();
				out_idx--;
				m_lists[prev].insert(m_lists[prev].end(), m_lists[out_idx].begin(), m_lists[out_idx].end());
				if (m_lists[prev].size() < TrunkSize * 2)
				{
					m_lists.erase(m_lists.begin() + out_idx);
				}
				else
				{
					m_lists[out_idx].clear();
					auto offset = m_lists.size() / 2;
					if (in_idx > offset)
						in_idx -= offset;
					m_lists[out_idx].insert(m_lists[out_idx].end(), m_lists[prev].begin() + offset, m_lists[prev].end());
					m_lists[prev].erase(m_lists[prev].begin() + m_lists.size() / 2, m_lists[prev].end());
					out_idx++;
				}
			}

		}

    };


    template <typename T, typename Compare = std::less<>>
    class sorted_set : public sorted_list<T, Compare, identity, true> { };


};

/*
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
#include <numeric>

class sorted_list_index_selector
{
public:

    template <typename I, typename Pred>
    void build(I first, I last, Pred pred)
    {
        std::inclusive_scan(first, last, std::back_inserter(m_index), [](const auto& x, const auto& y) {
            return x.size() + y.size();
        });
    }

    std::pair<std::size_t, std::size_t> index_of(std::size_t rank)
    {
        auto lower = std::lower_bound(m_index.begin(), m_index.end(), rank);
        if (lower == m_index.end())
            return { m_index.size(), 0 };
        if (*lower == rank) 
            return { std::distance(m_index.begin(), lower), *lower - *(lower - 1) };
        return { std::distance(m_index.begin(), lower), rank - *(lower - 1) };
    }

private:
    std::vector<std::size_t> m_index = { 0 };
};





// #include <assert.h>

// int main()
// {
//     std::vector values = { 1, 2, 3, 4, 5 };
//     std::vector<int> dest;
//     pair_wise_transform(values.begin(), values.end(), std::back_inserter(dest), std::plus<>());
//     for (auto val : dest)
//         std::cout << val << '\n';
// }



*/