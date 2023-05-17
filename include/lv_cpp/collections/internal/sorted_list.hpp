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

        template <bool Const> 
        struct sorted_list_iterator
        {
            using link_type = std::conditional_t<Const, const sorted_list*, sorted_list*>;
            
            using value_type = std::conditional_t<Const, const T, T>;
            using reference = value_type&;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::bidirectional_iterator_tag;

            link_type m_c;
            size_t m_out_idx;
            size_t m_in_idx;

            sorted_list_iterator() = default;

            sorted_list_iterator(const sorted_list_iterator&) = default;

            sorted_list_iterator(const sorted_list_iterator<!Const>& other) noexcept requires (Const)
                : m_c(other.m_c), m_out_idx(other.m_out_idx), m_in_idx(other.m_in_idx) { }

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

		using iterator = sorted_list_iterator<false>; 
		using const_iterator = sorted_list_iterator<true>; 
		using reverse_iterator = std::reverse_iterator<iterator>; 
		using const_reverse_iterator = std::reverse_iterator<const_iterator>; 
    
    private:
    
        using bucket_type = buffer<value_type, allocator_type>;
        using bucket_allocator_type = std::allocator_traits<allocator_type>::template rebind_alloc<bucket_type>;
        using bucket_list_type = buffer<bucket_type, bucket_allocator_type>;

        constexpr static bool IsAllocatorAlwaysEqual = typename std::allocator_traits<allocator_type>::is_always_equal();
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

        sorted_list(const sorted_list& other) = delete;
        sorted_list(const sorted_list& other, const allocator_type& alloc) = delete;
        
        sorted_list(sorted_list&& other, const allocator_type& alloc) = delete;
        sorted_list(sorted_list&& other) noexcept(NoexceptMoveConstruction) = delete;

        sorted_list& operator=(const sorted_list& other) = delete;
        sorted_list& operator=(sorted_list&& other) noexcept(NoexceptMoveAssignment) = delete;
        void swap(sorted_list& other) noexcept(NoexceptSwap) = delete;
        friend void swap(sorted_list& lhs, sorted_list& rhs) noexcept(NoexceptSwap) = delete;

		size_type size() const { return m_impl.m_size; }
		
		bool empty() const { return size() == 0; }

    private:
    public:

		// return lower_bound item
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
                return { buckets_ref().size(), 0 };
            }

            auto iter = std::ranges::lower_bound(*bucket, k, compare_ref(), KeyOfValue());

            return { 
                std::ranges::distance(buckets_ref().begin(), bucket),
                std::ranges::distance(bucket->begin(), iter)        
            };
		}

        bool expand(size_t pos)
        {
            if (buckets_ref().size() > TruckSize * 2)
            {
                auto& bucket = buckets_ref()[pos];
                bucket_list_type half;
                
                // The bucket_list_type will automatically check the iterator category
                // half.reserve(m_alloc, std::ranges::distance(bucket.begin() + TruckSize, bucket.end()));

                // Move the last part to half
                half.insert(m_alloc, half.end(),
                    make_move_iterator_if_noexcept(bucket.begin() + TruckSize),
                    make_move_iterator_if_noexcept(bucket.end())
                );

                bucket.erase(m_alloc, bucket.begin() + TruckSize, bucket.end());

                // half is trivially copyable
                buckets_ref().insert(m_alloc, buckets_ref().begin() + pos + 1, half);
                return true;
            }
            return false;
        }

#if 0
        template <typename U>
        std::pair<iterator, bool> insert_impl(U&& x)
        {
            bool succeed;
            size_t out_idx, in_idx;

            if (buckets_ref().size())
            {
                auto [i,  j] = find_item_by_key(KeyOfValue()(x));
                if (i == buckets_ref.size())
                {
                    buckets_ref().back().emplace_back(m_alloc, (U&&) x);
                    i = buckets_ref().size() - 1;
                    j = buckets_ref().back().size() - 1;
                    succeed = true;
                }
            }
            else
            {
                if (KeyOfValue()(buckets_ref()[i][j]) == KeyOfValue()(x))
                {
                    succeed = true;
                }
                else
                {
                    auto& v = buckets_ref()[i];
                    auto ret_iter = v.insert(m_alloc, v.begin() + j, (U&&) x);
                    j = std::ranges::distance(v.begin(), ret_iter);
                    succeed = true;
                }
            }

            bool expanded = expand(i);
            if (expanded && j > TruckSize)
            {
                i++;
                j -= TruckSize;
            }
            out_idx = i, in_idx = j;
        }
#endif

        auto& buckets_ref() const { return m_impl.m_buckets; }
        auto& buckets_ref() { return m_impl.m_buckets; }

        auto& compare_ref() const { return m_impl.m_cmp; }
        auto& compare_ref() { return m_impl.m_cmp; }

        sorted_list_impl m_impl;
        [[no_unique_address]] allocator_type m_alloc;
    };

    template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
    using sorted_set = sorted_list<std::tuple<T>, Compare, Allocator, identity, true>;

} // namespace leviathan::collections




