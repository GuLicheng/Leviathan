#pragma once

#include "common.hpp"

#include <algorithm>
#include <iostream>
#include <array>
#include <iterator>
#include <vector>
#include <memory_resource>
#include <type_traits>
#include <random>
#include <string>

#include <assert.h>

namespace std::pmr
{
    template<typename _Tp> class polymorphic_allocator;
} // name

namespace leviathan::collections
{

    template <typename T, typename Compare, typename Allocator, typename KeyOfValue, bool UniqueKey, int MaxLevel = 24, int Ratio = 4>
    class skip_list
    {
        static_assert(UniqueKey, "Not support multi-key now");

        inline static std::random_device rd;

        constexpr static std::random_device::result_type p = std::random_device::max() / Ratio;

        using node_allocator = std::allocator_traits<Allocator>::template rebind_alloc<char>;
        using node_alloc_traits = std::allocator_traits<node_allocator>;

        // static int get_level_debug()
        // {
        //     int level = 1;
        //     for (; rand() < RAND_MAX / Ratio; ++level);
        //     return std::min(MaxLevel, level);
        // }

        static int get_level()
        {
            // return get_level_debug(); // debug
            int level = 1;
            for (; rd() < p; ++level);
            return std::min(MaxLevel, level);
        }

        // https://en.wikipedia.org/wiki/Flexible_array_member
        struct skip_list_node 
        {
            alignas(T) unsigned char m_raw[sizeof(T)];
            int m_cnt;                 // size of next
            skip_list_node* m_prev;    // double recycle link list
            skip_list_node* m_next[];  // flexible array is forbidden in ISO C++.

            T* value_ptr()
            { return reinterpret_cast<T*>(m_raw); }

            const T* value_ptr() const
            { return reinterpret_cast<const T*>(m_raw); }

            void reset(skip_list_node* prev, skip_list_node* next)
            {
                m_prev = prev; 
                std::fill(m_next, m_next + m_cnt, next); 
            }

            static skip_list_node* allocate_node(node_allocator& alloc, int count)
            {
                const auto size = sizeof(skip_list_node) + count * sizeof(skip_list_node*);
                // For some fancy pointer such as FancyPtr<T>, the allocator may return 
                // FancyPtr<char>, so we cast the pointer to char* firstly.
                skip_list_node* node = reinterpret_cast<skip_list_node*>(
                    static_cast<char*>(node_alloc_traits::allocate(alloc, size))
                );
                node->m_cnt = count;
                return node;
            }

            static void deallocate(node_allocator& alloc, skip_list_node* node)
            {
                const auto size = sizeof(skip_list_node) + node->m_cnt * sizeof(skip_list_node*);
                node_alloc_traits::deallocate(alloc, reinterpret_cast<char*>(node), size);
            }

        };

        constexpr static bool IsTransparent = leviathan::collections::detail::is_transparent<Compare>;
        
        using Key = typename KeyOfValue::template type<T>;

        template <typename U> using key_arg_t = leviathan::collections::detail::key_arg<IsTransparent, U, Key>;


        template <bool Const>
        struct skip_list_iterator
        {

            using link_type = std::conditional_t<Const, const skip_list_node*, skip_list_node*>;
            using value_type = std::conditional_t<Const, const T, T>;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::bidirectional_iterator_tag;
            using reference = std::conditional_t<std::is_same_v<Key, T>, const value_type&, value_type&>;

            link_type m_ptr;

            constexpr skip_list_iterator() noexcept = default;
            constexpr skip_list_iterator(const skip_list_iterator&) noexcept = default;

            constexpr skip_list_iterator(const skip_list_iterator<!Const>& rhs) noexcept requires (Const)
                : m_ptr{ rhs.m_ptr } { }

            constexpr skip_list_iterator(link_type ptr) noexcept
                : m_ptr{ ptr } { }

            constexpr skip_list_iterator& operator++()
            {
                m_ptr = m_ptr->m_next[0];
                return *this;
            }

            constexpr skip_list_iterator& operator--()
            {
                m_ptr = m_ptr->m_prev;
                return *this;
            }

            constexpr skip_list_iterator operator++(int)
            {
                auto old = *this;
                ++* this;
                return old;
            }

            constexpr skip_list_iterator operator--(int)
            {
                auto old = *this;
                --* this;
                return old;
            }

            constexpr auto operator->() const
            { return std::addressof(this->operator*()); }

            constexpr reference operator*() const
            { return *(m_ptr->value_ptr()); }

            constexpr skip_list_iterator skip(difference_type i) const 
            { return { m_ptr->m_next[i] }; }

            constexpr skip_list_iterator& skip_to(difference_type i) 
            { return *this = skip(i); }

            constexpr void set_next(difference_type i, skip_list_iterator p)
            { m_ptr->m_next[i] = p.m_ptr; }

            constexpr void set_prev(skip_list_iterator p)
            { m_ptr->m_prev = p.m_ptr; }

            constexpr auto level() const 
            { return m_ptr->m_cnt; }

            constexpr bool operator==(const skip_list_iterator& rhs) const = default;

            skip_list_iterator<!Const> const_cast_to_iterator() const requires (Const)
            { return skip_list_iterator<!Const>(const_cast<typename skip_list_iterator<!Const>::link_type>(m_ptr)); }

        };

        constexpr static bool IsNothrowMoveConstruct = 
                    std::is_nothrow_move_constructible_v<Compare> 
                 && typename node_alloc_traits::is_always_equal();

        constexpr static bool IsNothrowMoveAssign = 
                    std::is_nothrow_move_assignable_v<Compare> 
                 && typename node_alloc_traits::is_always_equal();

        constexpr static bool IsNothrowSwap = 
                    std::is_nothrow_swappable_v<Compare> 
                 && typename node_alloc_traits::is_always_equal();

        using size_type = std::size_t;
        using allocator_type = Allocator;

        using iterator = skip_list_iterator<false>;
        using const_iterator = skip_list_iterator<true>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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

    public:

        skip_list() : skip_list(Compare(), Allocator()) { }

        explicit skip_list(const Compare& compare, const Allocator& allocator = Allocator())
            : m_cmp{ compare }, 
              m_alloc{ allocator },
              m_size{ 0 },
              m_level{ 1 } 
        {
            m_header = skip_list_node::allocate_node(m_alloc, MaxLevel);
            reset_header();
        }

        // TODO: ...
        skip_list(const skip_list&) = delete;
        skip_list(skip_list&&) noexcept(IsNothrowMoveConstruct) = delete;
        skip_list& operator=(const skip_list&) = delete;
        skip_list& operator=(skip_list&&) noexcept(IsNothrowMoveAssign) = delete;
        // void swap() noexcept(IsNothrowSwap);

        ~skip_list()
        {
            clear();
            clear_header();
        }


        // Iterators
        iterator begin() 
        { return { header()->m_next[0] }; }

        const_iterator begin() const 
        { return { header()->m_next[0] }; }

        iterator end() 
        { return { header() }; }

        const_iterator end() const 
        { return { header() }; }

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
        { return std::make_reverse_iterator(end()); }

        const_reverse_iterator rcend() const
        { return std::make_reverse_iterator(begin()); }

        // Capacity
        size_type size() const 
        { return m_size; }

        bool empty() const 
        { return size() == 0; }

        allocator_type get_allocator() const
        { return allocator_type(m_alloc); }

        size_type max_size() const 
        { return node_alloc_traits::max_size(m_alloc); }

        // Observers
        key_compare key_comp() const
        { return m_cmp; }

        value_compare value_comp() const
        { return m_cmp; }

        int current_level() const 
        { return m_level; }

        // Modifiers
        std::pair<iterator, bool> insert(const T& x)
        { return insert_unique(x); }

        std::pair<iterator, bool> insert(T&& x)
        { return insert_unique(std::move(x)); }

        iterator erase(const_iterator pos) 
        {
            auto ret = std::next(pos);
            erase_node_by_value(*pos);
            return ret.const_cast_to_iterator();
        }

        iterator erase(iterator pos)
        {
            auto ret = std::next(pos);
            erase_node_by_value(*pos);
            return ret;
        }

        iterator erase(iterator first, iterator last)
        {
            if (first == begin() && last == end()) 
                clear();
            else
                for (; first != last; first = erase(first));
            return last;
        }

        iterator erase(const_iterator first, const_iterator last)
        { return erase(first.const_cast_to_iterator(), last.const_cast_to_iterator()); }


        // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2077r3.html
        template <typename K> requires (IsTransparent)
        size_type erase(K&& x) 
        {
            auto old_size = size();
            erase_node_by_value(x);
            return old_size - size();
        }

        size_type erase(const key_type& x)
        {
            auto old_size = size();
            erase_node_by_value(x);
            return old_size - size();
        }


        // Lookup
        template <typename K = key_type>
        size_type count(const key_arg_t<K>& x)
        {
            auto [lower, upper] = equal_range(x);
            return std::distance(lower, upper);
        }

        template <typename K = key_type>
        iterator find(const key_arg_t<K>& x)
        {
            auto [node, exist] = find_node(x);
            return exist ? iterator(node) : end();
        }

        template <typename K = key_type>
        const_iterator find(const key_arg_t<K>& x) const
        { return const_cast<skip_list&>(*this).find(x); }

        template <typename K = key_type>
        bool contains(const key_arg_t<K>& x) const
        { return find(x) != end(); }

        template <typename K = key_type>
        iterator lower_bound(const key_arg_t<K>& x)
        {
            auto [node, exist] = find_node(x);
            return exist ? iterator(node) : std::next(iterator(node));
        }

        template <typename K = key_type>
        const_iterator lower_bound(const key_arg_t<K>& x) const
        { return const_cast<skip_list&>(*this).lower_bound(x); }

        template <typename K = key_type>
        iterator upper_bound(const key_arg_t<K>& x)
        {
            auto [node, exist] = find_node(x);
            return std::next(iterator(node));
        }

        template <typename K = key_type>
        const_iterator upper_bound(const key_arg_t<K>& x) const
        { return const_cast<skip_list&>(*this).upper_bound(x); }

        template <typename K = key_type>
        std::pair<iterator, iterator> equal_range(const key_arg_t<K>& x)
        {
            auto [node, exist] = find_node(x);
            auto lower = iterator(node);
            auto upper = std::next(lower);
            return exist ? 
                std::pair<iterator, iterator>{ lower, upper } : 
                std::pair<iterator, iterator>{ upper, upper };
        }

        template <typename K = key_type>
        std::pair<const_iterator , const_iterator> equal_range(const key_arg_t<K>& x) const
        { return const_cast<skip_list&>(*this).equal_range(x); }

        void show() const
        {
            constexpr auto width = 5;
            std::cout << "Header: " << header()->m_cnt << " Level: " << current_level() << '\n';
            for (auto iter = begin(); iter != end(); ++iter)
            {
                std::string ss = std::to_string(*iter);
                if (ss.size() < width)
                    ss.append(width - ss.size(), ' ');
                std::cout << "Value = " << ss << " (" << iter.m_ptr->m_cnt << ")";
                for (int i = 0; i < m_level; ++i)
                {
                    std::string s = std::to_string(*iter);
                    if (s.size() < width)
                        s.append(width - s.size(), ' ');
                    if (i < iter.m_ptr->m_cnt)
                        std::cout << " -> " << s.substr(0, width);
                    else if (header()->m_next[i] != iter.m_ptr)
                        std::cout << "    " << "|    ";
                }
                std::cout << "\n";
            }
            std::cout << "\n"; 
        }


        friend auto operator<=>(const skip_list& lhs, const skip_list& rhs)
        {
            return std::lexicographical_compare_three_way(
                lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        void clear()
        { reset(); }

    private:

        void reset()
        {
            auto node = m_header->m_next[0];
            while (node != m_header)
            {
                auto next = node->m_next[0];
                drop_node(node);
                node = next;
            }
            m_level = 1;
            m_size = 0;
            reset_header();
        }

        void clear_header()
        { put_node(m_header); }

        void reset_header()
        {
            assert(header()->m_cnt == MaxLevel);
            header()->reset(header(), header());
        }

        template <typename U>
        std::pair<iterator, bool> insert_unique(U&& val)
        {
            std::array<iterator, MaxLevel> prev;
            prev.fill(end());
            auto [cur, exist] = find_node_with_prev(KeyOfValue()(val), prev);

            if (exist)
                return { cur, false };

            // insert a new node
            auto new_node = create_node((U&&) val);
            new_node->reset(cur.m_ptr, header());
            cur.skip(0).set_prev(new_node);
            iterator worker { new_node };
            iterator head { header() };
            
            // update header
            const int level = new_node->m_cnt;
            for (int i = 0; i < level; ++i)
            {
                if (i >= m_level)
                    head.set_next(i, worker);
                else
                {
                    worker.set_next(i, prev[i].skip(i));
                    prev[i].set_next(i, worker);
                }
            }
            m_level = std::max(m_level, level);
            ++m_size;
            return { worker, true };
        }

        // return target node if succeed otherwise prev position of target node for inserting
        template <typename K>
        std::pair<iterator, bool> find_node_with_prev(const K& val, std::array<iterator, MaxLevel>& prev) 
        {
            iterator cur = header();
            iterator sent = end();
            bool exits = false;
            for (int i = m_level - 1; i >= 0; --i)
            {
                for (; cur.skip(i) != sent && m_cmp(KeyOfValue()(*cur.skip(i)), val); cur.skip_to(i));
                auto next = cur.skip(i);
                // cur is prev of position, so val <= *next
                if (next != end() && !m_cmp(val, KeyOfValue()(*next)))
                {
                    exits = true; // find it and do nothing
                }
                prev[i] = cur;
            }
            // cur is prev of node
            return { cur, exits };
        }

        // return target node if succeed otherwise prev of target value 
        template <typename K>
        std::pair<iterator, bool> find_node(const K& val) 
        {
            auto cur = iterator(header());
            auto sent = end();
            for (int i = m_level; i >= 0; --i)
            {
                for (; cur.skip(i) != sent && m_cmp(KeyOfValue()(*cur.skip(i)), val); cur.skip_to(i));
                auto next = cur.skip(i);
                if (next != end() && !m_cmp(val, KeyOfValue()(*next)))
                {
                    return { next, true };
                }
            }
            return { cur, false };
        }

        template <typename K>
        void erase_node_by_value(const K& val)
        {
            std::array<iterator, MaxLevel> prev;
            prev.fill(end());
            auto [cur, exist] = find_node_with_prev(val, prev);

            if (!exist)
                return;

            auto deleted_pos = cur.skip(0);
            deleted_pos.skip(0).set_prev(cur);
            for (int i = 0; i < deleted_pos.level(); ++i)
                prev[i].set_next(i, deleted_pos.skip(i));

            int new_level;
            iterator sent = end();
            iterator head = iterator{ header() };
            for (new_level = m_level - 1; new_level >= 0 && head.skip(new_level) == sent; --new_level);
            m_level = new_level + 1;

            drop_node(deleted_pos.m_ptr);
            --m_size;
            return;
        }


        skip_list_node* get_node(int count)
        {
            skip_list_node* node = skip_list_node::allocate_node(m_alloc, count);
            return node;
        }

        template <typename... Args>
        void construct_node(skip_list_node* node, Args&&... args)
        {
            try
            {
                node_alloc_traits::construct(m_alloc, node->value_ptr(), (Args&&) args...);
            }
            catch (...)
            {
                node_alloc_traits::destroy(m_alloc, node->value_ptr());
                put_node(node);
                throw;
            }
        }

        void put_node(skip_list_node* p)
        { skip_list_node::deallocate(m_alloc, p); }

        template <typename... Args>
        skip_list_node* create_node(Args&&... args)
        {
            skip_list_node* tmp = get_node(get_level());
            construct_node(tmp, (Args&&) args...);
            return tmp;
        }

        void destroy_node(skip_list_node* p)
        { node_alloc_traits::destroy(m_alloc, p->value_ptr()); }

        void drop_node(skip_list_node* p)
        {
            destroy_node(p);
            put_node(p);
        }

        skip_list_node* header()
        { return m_header; }

        const skip_list_node* header() const 
        { return m_header; }

        // members
        [[no_unique_address]] Compare m_cmp;
        [[no_unique_address]] node_allocator m_alloc;
        std::size_t m_size;
        skip_list_node* m_header;
        int m_level;

    };

    template <typename T, typename Compare = std::less<>, typename Allocator = std::allocator<T>>
    class skip_set : public skip_list<T, Compare, Allocator, identity, true> { };


}

