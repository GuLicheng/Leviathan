#pragma once

#include "common.hpp"
#include "buffer.hpp"
#include "../../utils/iterator.hpp"

#include <type_traits>

namespace leviathan::collections
{
    template <typename TypePack, 
        typename Compare, 
        typename Allocator, 
        typename KeyOfValue, 
        bool UniqueKey, size_t TruckSize = 1024>
    class sorted_list
    {

        using T = typename KeyOfValue::template value_type<TypePack>;

        struct sorted_list_iterator
        {
            using link_type = sorted_list*;
            
            using value_type = T;
            using reference = value_type&;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::bidirectional_iterator_tag;

            link_type m_c;
            size_t m_out_idx;
            size_t m_in_idx;

            sorted_list_iterator() = default;

            sorted_list_iterator(const sorted_list_iterator&) = default;

            sorted_list_iterator(link_type c , size_t out_idx, size_t in_idx = 0)
                : m_c(c), m_out_idx(out_idx), m_in_idx(in_idx) { }

            bool operator==(const sorted_list_iterator& other) const = default;

            reference operator*() const { return m_c->buckets_ref()[m_out_idx][m_in_idx]; }

            auto operator->() const { return std::addressof(this->operator*()); }
        
            sorted_list_iterator& operator++()
            {
                m_in_idx++;
                if (m_in_idx == m_c->buckets_ref()[m_out_idx].size())
                    m_out_idx++, m_in_idx = 0;
                return *this;
            }

            sorted_list_iterator operator++(int)
            {
				auto old = *this;
				++ *this;
				return old;
            }

            sorted_list_iterator& operator--()
            {
                if (m_in_idx == 0)
                    m_in_idx = m_c->buckets_ref()[--m_out_idx].size();
                m_in_idx--;
                return *this;
            }

            sorted_list_iterator operator--(int)
            {
				auto old = *this;
				-- *this;
				return old;
            }
        };

    public:

        using value_type = T;
        using key_type = typename KeyOfValue::template key_type<TypePack>;
        using allocator_type = Allocator;
        using size_type = size_t;

		using iterator = sorted_list_iterator; 
		using const_iterator = std::const_iterator<iterator>; 
		using reverse_iterator = std::reverse_iterator<iterator>; 
		using const_reverse_iterator = std::reverse_iterator<const_iterator>; 
    
    private:

        using alloc_traits = std::allocator_traits<allocator_type>;
        using bucket_type = buffer<value_type, allocator_type>;
        using bucket_allocator_type = alloc_traits::template rebind_alloc<bucket_type>;
        using bucket_list_type = buffer<bucket_type, bucket_allocator_type>;
        using self_type = sorted_list;

        constexpr static bool IsTransparent = detail::is_transparent<Compare>;
        
        template <typename U>
        using key_arg_t = detail::key_arg<IsTransparent, U, key_type>;

        constexpr static bool IsAllocatorAlwaysEqual = typename alloc_traits::is_always_equal();
        constexpr static bool NoexceptSwap = std::is_nothrow_swappable_v<Compare> && IsAllocatorAlwaysEqual;
        constexpr static bool NoexceptMoveConstruction = std::is_nothrow_move_constructible_v<Compare> && IsAllocatorAlwaysEqual;
        constexpr static bool NoexceptMoveAssignment = std::is_nothrow_assignable_v<Compare> && IsAllocatorAlwaysEqual;

        struct sorted_list_impl
        {
            [[no_unique_address]] Compare m_cmp;
            size_t m_size = 0; 
            bucket_list_type m_buckets;

            sorted_list_impl(const Compare& compare = Compare()) : m_cmp(compare) { }

            void swap(sorted_list_impl& other) noexcept(std::is_nothrow_swappable_v<Compare>)
            {
                using std::swap;
                swap(m_cmp, other.m_cmp);
                swap(m_size, other.m_size);
                m_buckets.swap(other.m_buckets);
            }
        };

    public:

        sorted_list(const Compare& compare, const allocator_type& alloc) : m_impl(compare), m_alloc(alloc) { }

        sorted_list() : sorted_list(Compare(), allocator_type()) { }

        // TODO
        sorted_list(const sorted_list& other) = delete;
        sorted_list(const sorted_list& other, const allocator_type& alloc) = delete;
        
        sorted_list(sorted_list&& other, const allocator_type& alloc) = delete;
        sorted_list(sorted_list&& other) noexcept(NoexceptMoveConstruction) = delete;

        sorted_list& operator=(const sorted_list& other) = delete;
        sorted_list& operator=(sorted_list&& other) noexcept(NoexceptMoveAssignment) = delete;
        void swap(sorted_list& other) noexcept(NoexceptSwap) = delete;
        friend void swap(sorted_list& lhs, sorted_list& rhs) noexcept(NoexceptSwap) = delete;

        ~sorted_list() 
        { clear(); }

		size_type size() const 
        { return m_impl.m_size; }
		
		bool empty() const 
        { return size() == 0; }

		iterator begin()  
		{ return { this, 0 }; }
		
		iterator end()  
		{ return { this, buckets_size() }; }

		const_iterator begin() const  
		{ return const_cast<self_type&>(*this).begin(); }

		const_iterator end() const  
		{ return const_cast<self_type&>(*this).end(); }

		const_iterator cbegin() const  
		{ return begin(); }

		const_iterator cend() const  
		{ return end(); }

		reverse_iterator rbegin()  
		{ return end(); } 

		reverse_iterator rend()  
		{ return begin(); } 

		const_reverse_iterator rbegin() const  
		{ return end(); } 

		const_reverse_iterator rend() const  
		{ return begin(); } 
		
		const_reverse_iterator rcbegin() const  
		{ return rbegin(); } 

		const_reverse_iterator rcend() const  
		{ return rend(); } 

        // insert
        template <typename... Args>
        std::pair<iterator, bool> emplace(Args&&... args)
        { 
            if constexpr (detail::emplace_helper<value_type, Args...>::value)
            {
                return insert_impl((Args&&) args...);
            }
            else
            {
                value_handle<value_type, allocator_type> handle(m_alloc, (Args&&) args...);
                return insert_impl(*handle);
            }
        }

		std::pair<iterator, bool> insert(const value_type& x) 
		{ return emplace(x); }

		std::pair<iterator, bool> insert(value_type&& x) 
		{ return emplace(std::move(x)); }

		iterator insert(const_iterator, const value_type& x) 
		{ return insert_impl(x).first; }

		iterator insert(const_iterator, value_type&& x) 
		{ return insert_impl(std::move(x)).first; }

        // search
        template <typename K = key_type>
        bool contains(const key_arg_t<K>& x) 
        { return find(x) != end(); }

        template <typename K = key_type>
        bool contains(const key_arg_t<K>& x) const
        { return const_cast<self_type&>(*this).contains(x); }

        template <typename K = key_type>
        iterator find(const key_arg_t<K>& x) 
        {
            auto it = lower_bound(x);
            if (it == end() || !check_equal(x, it))
                return end();
            return it;
        }

        template <typename K = key_type>
        const_iterator find(const key_arg_t<K>& x) const
        { return const_cast<self_type&>(*this).find(x); }

        template <typename K = key_type>
        iterator lower_bound(const key_arg_t<K>& x) 
        { return lower_bound_impl(x); } 

        template <typename K = key_type>
        const_iterator lower_bound(const key_arg_t<K>& x) const
        { return const_cast<self_type&>(*this).lower_bound(x); } 

        template <typename K = key_type>
        iterator upper_bound(const key_arg_t<K>& x) 
        {
            auto it = lower_bound(x);
            if (it == end()) 
                return it;
            return check_equal(x, it) ? std::ranges::next(it) : it;
        } 

        template <typename K = key_type>
        const_iterator upper_bound(const key_arg_t<K>& x) const
        { return const_cast<self_type&>(*this).upper_bound(x); } 

        template <typename K = key_type>
        std::pair<iterator, iterator> equal_range(const key_arg_t<K>& x) 
        { 
            auto it = lower_bound(x);
            if (it == end()) 
                return { it, it };
            return { it, check_equal(x, it) ? std::ranges::next(it) : it };
        } 

        template <typename K = key_type>
        std::pair<const_iterator, const_iterator> equal_range(const key_arg_t<K>& x) const
        { return const_cast<self_type&>(*this).equal_range(x); } 

        // remove
		iterator erase(const_iterator pos) 
		{ return remove_impl(pos); }

		iterator erase(iterator pos) 
		{ return remove_impl(pos); }

        size_type erase(const key_type& x) 
		{ return remove_impl(x); }

		// template <typename K> size_type erase(K&& x);

        void clear()
        { clear_buckets(); }

        size_t buckets_size() const
        { return buckets_ref().size(); }


    private:
    // public:

        void clear_buckets()
        {
            // Call dtor for each inner_bucket.
            for (int i = 0; i < buckets_size(); ++i)
            {
                buckets_ref()[i].dispose(m_alloc);
            }
            // Call dtor for outer buffer.
            bucket_allocator_type ba(m_alloc);
            buckets_ref().dispose(ba);
        }

		iterator remove_impl(const_iterator cpos) 
		{
            // Find and remove the element.
            iterator pos = cpos.base();
			auto i = pos.m_out_idx, j = pos.m_in_idx;
            buckets_ref()[i].erase(m_alloc, buckets_ref()[i].begin() + j);
			--m_impl.m_size;

            if (buckets_ref()[i].empty())
			{
                bucket_allocator_type ba(m_alloc);
				buckets_ref().erase(ba, buckets_ref().begin() + i);
				return { this, i, j };
			}
			shrink(i);	

            // If the removed element is at the end of bucket, them
            // return the first position of next bucket.
			if (j == buckets_ref()[i].size())
				return { this, i + 1, 0 };

			return { this, i, j };
		}

		size_type remove_impl(const key_type& x) 
		{
			auto pos = find(x);
			if (pos == end())
				return 0;
			erase(pos);
			return 1;
		}

		// Make sure that each bucket will not be empty!
		void shrink(size_t pos)
		{
            assert(buckets_ref()[pos].size() && "bucket will not be empty!");

            // If there is only one bucket, shrink
            if (buckets_ref()[pos].size() < TruckSize / 2 && buckets_size() > 1)
            {
                if (pos == 0) pos++;

                auto prev = pos - 1;

                auto& prev_bucket = buckets_ref()[prev];
                auto& next_bucket = buckets_ref()[pos];

                // Try merge two buckets.

                // Move the second bucket to first bucket.
                prev_bucket.insert(m_alloc, prev_bucket.end(), next_bucket.begin(), next_bucket.end());

                // If the number of elements in first bucket is not enough, remove
                // the second bucket, otherwise, move half of elements to the 
                // second bucket.
                if (prev_bucket.size() < TruckSize * 2)
                {
                    bucket_allocator_type ba(m_alloc);
                    buckets_ref().erase(ba, buckets_ref().begin() + pos);
                }
                else
                {
                    next_bucket.clear(m_alloc);
                    next_bucket.insert(
                        m_alloc, 
                        next_bucket.end(),
                        make_move_iterator_if_noexcept(prev_bucket.begin() + prev_bucket.size() / 2),
                        make_move_iterator_if_noexcept(prev_bucket.end())
                    );

                    prev_bucket.erase(
                        m_alloc,  
                        prev_bucket.begin() + prev_bucket.size() / 2, 
                        prev_bucket.end()
                    );
                }
            }

		}

        // Check whether the *pos is equal to x.
        template <typename K>
        bool check_equal(const K& x, iterator pos)
        {
            // Assume the Compare is std::less, and *pos >= x.
            // If m_cmp(x, *pos) is true, then the x is less than *pos.
            return !compare_ref()(x, *pos);
        }

		// return lower_bound item.
		template <typename K>
		std::pair<std::size_t, std::size_t> find_item_by_key(const K& k) const 
		{
            auto bucket = std::ranges::lower_bound(buckets_ref(), k, compare_ref(), 
                [this](const auto& b) -> auto& { 
                    // b is bucket
                    assert(b.size() && "b shoule not be empty!");
                    return KeyOfValue()(b.back()); 
                }
            );

            if (bucket == buckets_ref().cend())
            {
                return { buckets_size(), 0 };
            }

            auto iter = std::ranges::lower_bound(*bucket, k, compare_ref(), KeyOfValue());

            return { 
                std::ranges::distance(buckets_ref().begin(), bucket),
                std::ranges::distance(bucket->begin(), iter)        
            };
		}

        /**
         * @brief Split bucket at pos
         * @return True if the number of elements in pos bucket is sufficient quantity.
        */
        bool try_expand(size_t pos)
        {
            auto& bucket = buckets_ref()[pos];

            if (bucket.size() > TruckSize * 2)
            {
                bucket_type half;
                
                // Move the last part to half, the bucket_list_type will automatically check the iterator category.
                half.insert(m_alloc, half.end(),
                    make_move_iterator_if_noexcept(bucket.begin() + TruckSize),
                    make_move_iterator_if_noexcept(bucket.end())
                );

                bucket.erase(m_alloc, bucket.begin() + TruckSize, bucket.end());

                // half is trivially copyable
                bucket_allocator_type balloc(m_alloc);
                buckets_ref().emplace(balloc, buckets_ref().begin() + pos + 1, half);
                return true;
            }
            return false;
        }

        template <typename U>
        std::pair<iterator, bool> insert_impl(U&& val)
        {
			bool succeed;
			std::size_t out_idx, in_idx;

			if (buckets_size())
			{
				auto [i, j] = find_item_by_key(KeyOfValue()(val));
				if (i == buckets_size())
				{
					// insert last position
					buckets_ref().back().emplace_back(m_alloc, (U&&) val);
					i = buckets_size() - 1;
					j = buckets_ref().back().size() - 1;
					succeed = true;
				}
				else
				{
                    // (i, j) >= value => if (value < (i, j)) then value is not exist
					if (!compare_ref()(KeyOfValue()(val), KeyOfValue()(buckets_ref()[i][j])))
					{
						succeed = false;
					}
					else
					{
						auto& v = buckets_ref()[i];
						auto ret_iter = v.emplace(m_alloc, v.begin() + j, (U&&)val);
						j = std::ranges::distance(v.begin(), ret_iter);
						succeed = true;
					}
				}

				bool expanded = try_expand(i);
				if (expanded && j > TruckSize)
				{
					i++;
					j -= TruckSize;
				}
				out_idx = i, in_idx = j;
			}
			else
			{
                bucket_type b;
                b.emplace_back(m_alloc, (U&&) val);
                bucket_allocator_type ba(m_alloc);
				buckets_ref().emplace_back(ba, b);
				out_idx = in_idx = 0;
				succeed = true;
			}
			if (succeed) m_impl.m_size++;
			return { iterator(this, out_idx, in_idx), succeed };
        }

		template <typename K>
		iterator lower_bound_impl(const K& val) 
		{
			auto [i, j] = find_item_by_key(val);
			return i == buckets_size() ? end() : iterator(this, i, j);
		}

		// template <typename K>
		// iterator upper_bound_impl(const K& val) noexcept
		// {
		// 	auto lower = lower_bound_impl(val);
		// 	return std::find_if(lower, end(), [&](const auto& x){
		// 		return Config::get_key(x) != val;
		// 	});
		// }

        auto& buckets_ref() const 
        { return m_impl.m_buckets; }

        auto& buckets_ref() 
        { return m_impl.m_buckets; }

        auto& compare_ref() const 
        { return m_impl.m_cmp; }

        auto& compare_ref() 
        { return m_impl.m_cmp; }

        sorted_list_impl m_impl;
        [[no_unique_address]] allocator_type m_alloc;
    };

    template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
    using sorted_set = sorted_list<std::tuple<T>, Compare, Allocator, identity, true>;

} // namespace leviathan::collections




