#pragma once

#include "common.hpp"

#include <assert.h>
#include <iostream>
#include <memory>
#include <utility>

namespace std::pmr
{
    template<typename _Tp> class polymorphic_allocator;
} // namespace std::pmr

namespace leviathan::collections 
{

    struct avl_node_base
    {
        avl_node_base* m_parent;
        avl_node_base* m_left;
        avl_node_base* m_right;
        int m_height; // header is -1. 
        // For a empty set in gcc or clang, forward it's iterator will cause infinity loop such:
        // std::set<int> s; s.begin()++;

        constexpr void reset()
        {
            m_parent = nullptr;
            m_left = m_right = this;
            m_height = -1;
        }

        constexpr static avl_node_base* increment(avl_node_base* x)
        {
            assert(x && "x should not be nullptr");
            if (x->m_right)
            {
                x = minimum(x->m_right);
            }
            else
            {
                avl_node_base* y = x->m_parent;
                while (x == y->m_right)
                {
                    x = y;
                    y = y->m_parent;
                }
                if (x->m_right != y)
                    x = y;
            }
            return x;
        }

        constexpr static avl_node_base* decrement(avl_node_base* x)
        {
            assert(x && "x should not be nullptr");
            if (x->m_left)
            {
                x = maximum(x->m_left);
            }
            else
            {
                avl_node_base* y = x->m_parent;
                while (x == y->m_left)
                {
                    x = y;
                    y = y->m_parent;
                }
                if (x->m_left != y)
                    x = y;
            }
            return x;
        }

        constexpr static const avl_node_base* increment(const avl_node_base* x)
        { return increment(const_cast<avl_node_base*>(x)); }

        constexpr static const avl_node_base* decrement(const avl_node_base* x)
        { return decrement(const_cast<avl_node_base*>(x)); }

        constexpr static avl_node_base* maximum(avl_node_base* x)
        {
            assert(x && "x should not be nullptr");
            for (; x->m_right; x = x->m_right);
            return x;
        }

        constexpr static avl_node_base* minimum(avl_node_base* x)
        {
            assert(x && "x should not be nullptr");
            for (; x->m_left; x = x->m_left);
            return x;
        }

        constexpr static const avl_node_base* maximum(const avl_node_base* x)
        { return maximum(const_cast<avl_node_base*>(x)); }

        constexpr static const avl_node_base* minimum(const avl_node_base* x)
        { return minimum(const_cast<avl_node_base*>(x)); }

        constexpr static int height(const avl_node_base* x)
        { return x ? x->m_height : 0; }

        /*
        *     x            y              
        *       \   =>   /    
        *         y    x
        */
        constexpr static void tree_rotate_left(avl_node_base* x, avl_node_base*& root)
        {
            avl_node_base* y = x->m_right;

            x->m_right = y->m_left;
            if (y->m_left != 0)
                y->m_left->m_parent = x;
            y->m_parent = x->m_parent;

            // x->parent will never be nullptr, since header->parent == root and root->parent == header
            if (x == root)
                root = y;
            else if (x == x->m_parent->m_left) 
                x->m_parent->m_left = y;
            else
                x->m_parent->m_right = y;
            y->m_left = x;
            x->m_parent = y;
        }


        /*
        *     x        y                   
        *    /     =>    \
        *   y              x
        */
        constexpr static void tree_rotate_right(avl_node_base* x, avl_node_base*& root)
        {
            avl_node_base* y = x->m_left;

            x->m_left = y->m_right;
            if (y->m_right != 0)
                y->m_right->m_parent = x;
            y->m_parent = x->m_parent;

            if (x == root)
                root = y;
            else if (x == x->m_parent->m_right)
                x->m_parent->m_right = y;
            else
                x->m_parent->m_left = y;
            y->m_right = x;
            x->m_parent = y;
        }

        constexpr static void update_height(avl_node_base* x)
        {
            assert(x && "x should not be nullptr");
            int lh = height(x->m_left);
            int rh = height(x->m_right);
            x->m_height = std::max(lh, rh) + 1;
        }

        // remove x from tree and return removed node
        constexpr static void erase_node(avl_node_base* x, avl_node_base* header)
        {
            assert(x && "x should not be nullptr");

            avl_node_base*& root = header->m_parent;
            avl_node_base*& leftmost = header->m_left;
            avl_node_base*& rightmost = header->m_right;

            avl_node_base* child = nullptr;
            avl_node_base* parent = nullptr; // for rebalance

            if (x->m_left && x->m_right)
            {
                auto successor = minimum(x->m_right);
                child = successor->m_right;
                parent = successor->m_parent;
                if (child)
                {
                    child->m_parent = parent;
                }

                    (successor->m_parent->m_left == successor ? 
                    successor->m_parent->m_left : 
                    successor->m_parent->m_right) = child;


                if (successor->m_parent == x)
                    parent = successor;
                
                successor->m_left = x->m_left;
                successor->m_right = x->m_right;
                successor->m_parent = x->m_parent;
                successor->m_height = x->m_height;
            
                if (x == root)
                    root = successor;
                else
                    (x->m_parent->m_left == x ? x->m_parent->m_left : x->m_parent->m_right) = successor;
                
                x->m_left->m_parent = successor;

                if (x->m_right)
                    x->m_right->m_parent = successor;

            }
            else
            {
                // update leftmost or rightmost
                if (!x->m_left && !x->m_right) 
                {
                    // leaf, such as just one root
                    if (x == leftmost)
                        leftmost = x->m_parent;
                    if (x == rightmost)
                        rightmost = x->m_parent;
                }
                else if (x->m_left)
                {
                    // only left child
                    child = x->m_left;
                    if (x == rightmost)
                        rightmost = maximum(child);
                }                
                else
                {
                    // only right child
                    child = x->m_right;
                    if (x == leftmost)
                        leftmost = minimum(child);
                }

                if (child)
                    child->m_parent = x->m_parent;
                if (x == root)
                    root = child;
                else
                    (x->m_parent->m_left == x ? x->m_parent->m_left : x->m_parent->m_right) = child;
                parent = x->m_parent;
            }
            avl_tree_rebalance_erase(parent, header);
        }


        constexpr static void avl_tree_fix_l(avl_node_base* x, avl_node_base* header)
        {
            auto r = x->m_right;
            int lh0 = height(r->m_left);
            int rh0 = height(r->m_right);
            if (lh0 > rh0)
            {
                tree_rotate_right(r, header->m_parent);
                update_height(r);
                update_height(r->m_parent);
            }
            tree_rotate_left(x, header->m_parent);
            update_height(x);
            update_height(x->m_parent);
        }

        constexpr static void avl_tree_fix_r(avl_node_base* x, avl_node_base* header)
        {
            auto l = x->m_left;
            int lh0 = height(l->m_left);
            int rh0 = height(l->m_right);
            if (lh0 < rh0)
            {
                tree_rotate_left(l, header->m_parent);
                update_height(l);
                update_height(l->m_parent);
            }
            tree_rotate_right(x, header->m_parent);
            update_height(x);
            update_height(x->m_parent);
        }

        constexpr static void avl_tree_rebalance_insert(avl_node_base* x, avl_node_base* header)
        {
            for (x = x->m_parent; x != header; x = x->m_parent)
            {
                int lh = height(x->m_left);
                int rh = height(x->m_right);
                int h = std::max(lh, rh) + 1;
                if (x->m_height == h) break;
                x->m_height = h;

                int diff = lh - rh;
                if (diff <= -2)
                {
                    avl_tree_fix_l(x, header);
                }
                else if (diff >= 2)
                {
                    avl_tree_fix_r(x, header);
                }
            }
        }

        constexpr static void avl_tree_rebalance_erase(avl_node_base* x, avl_node_base* header)
        {
            for (; x != header; x = x->m_parent)
            {
                int lh = height(x->m_left);
                int rh = height(x->m_right);
                int h = std::max(lh, rh) + 1;
                x->m_height = h;

                int diff = lh - rh;
                
                if (x->m_height != h)
                    x->m_height = h;
                else if (-1 <= diff && diff <= 1) 
                    break;

                if (diff <= -2)
                    avl_tree_fix_l(x, header);
                else 
                    avl_tree_fix_r(x, header);
            }
        }

        constexpr static void avl_tree_insert_and_rebalance(bool insert_left,
                                      avl_node_base* x,
                                      avl_node_base* p,
                                      avl_node_base& header)
        {
            x->m_parent = p;
            x->m_left = x->m_right = nullptr;
            x->m_height = 1;

            if (insert_left)
            {
                p->m_left = x;
                if (p == &header)
                {
                    header.m_parent = x;
                    header.m_right = x;
                }
                else if (p == header.m_left)
                    header.m_left = x;
            }
            else
            {
                p->m_right = x;
                if (p == header.m_right)
                    header.m_right = x;
            }

            // rebalance
            avl_node_base::avl_tree_rebalance_insert(x, &header);
        }

        constexpr static avl_node_base* avl_tree_rebalance_for_erase(avl_node_base* z, avl_node_base& header)
        {
            erase_node(z, std::addressof(header));
            return z;
        }

    };

    template <typename T, typename Compare, typename Allocator, typename KeyOfValue, bool UniqueKey>
    class avl_tree
    {
        static_assert(UniqueKey, "Not Support MultiKey");
        
        struct avl_node : avl_node_base
        {
            T m_val;
            // alignas(T) unsigned char m_val[sizeof(T)];

            T* value_ptr() 
            { return reinterpret_cast<T*>(std::addressof(m_val)); }

            const T* value_ptr() const
            { return reinterpret_cast<const T*>(std::addressof(m_val)); }

        };


        using node_allocator = std::allocator_traits<Allocator>::template rebind_alloc<avl_node>;
        using node_alloc_traits = std::allocator_traits<node_allocator>;

        constexpr static bool IsTransparent = leviathan::collections::detail::is_transparent<Compare>;
        
        using Key = typename KeyOfValue::template type<T>;

        template <typename U> using key_arg_t = leviathan::collections::detail::key_arg<IsTransparent, U, Key>;

        template <bool Const>
        struct tree_iterator
        {
            using link_type = std::conditional_t<Const, const avl_node_base*, avl_node_base*>;
            using value_type = std::conditional_t<Const, const T, T>;
            using reference = value_type&;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::bidirectional_iterator_tag;

            link_type m_ptr;

            constexpr tree_iterator() noexcept = default;
            constexpr tree_iterator(const tree_iterator&) noexcept = default;

            constexpr tree_iterator(const tree_iterator<!Const>& rhs) noexcept requires (Const)
                : m_ptr{ rhs.m_ptr } { }

            constexpr tree_iterator(link_type ptr) noexcept
                : m_ptr{ ptr } { }

            constexpr tree_iterator& operator++()
            {
                // if m_ptr->m_height == -1, this node must be header/end/sentinel, we simply make it cycle
                m_ptr = m_ptr->m_height == -1 ? m_ptr->m_left : avl_node_base::increment(m_ptr);
                return *this;
            }

            constexpr tree_iterator operator++(int)
            {
                auto old = *this;
                ++*this;
                return old;
            } 

            constexpr tree_iterator& operator--()
            {
                m_ptr = m_ptr->m_height == -1 ? m_ptr->m_right : avl_node_base::decrement(m_ptr);
                return *this;
            }

            constexpr tree_iterator operator--(int)
            {
                auto old = *this;
                --*this;
                return old;
            } 

            constexpr auto& operator*() const 
            {
                using cast_link_type = std::conditional_t<Const, const avl_node*, avl_node*>;  
                return *(static_cast<cast_link_type>(m_ptr)->value_ptr()); 
            }

            constexpr auto operator->() const
            { return std::addressof(this->operator*()); }
            
            constexpr bool operator==(const tree_iterator&) const = default;

            tree_iterator<!Const> const_cast_to_iterator() const requires (Const)
            { return tree_iterator<!Const>(const_cast<typename tree_iterator<!Const>::link_type>(m_ptr)); }

        };


    public:
        
        using tree_node = avl_node;
        using size_type = std::size_t;
        using allocator_type = Allocator;

        using iterator = tree_iterator<false>;
        using const_iterator = tree_iterator<true>;
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


        avl_tree()
        {
            m_header.reset();
            m_size = 0;
        }

        ~avl_tree()
        { clear(); }

        // Iterators
        iterator begin() 
        { return { m_header.m_left }; }

        const_iterator begin() const 
        { return { m_header.m_left }; }

        iterator end() 
        { return { &m_header }; }

        const_iterator end() const 
        { return { &m_header }; }

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

        // Modifiers
        std::pair<iterator, bool> insert(const value_type& x)
        { return emplace(x); }

        std::pair<iterator, bool> insert(value_type&& x)
        { return emplace(std::move(x)); }

        iterator insert(const_iterator hint, const value_type& x)
        { return emplace_hint(hint, x); }

        iterator insert(const_iterator hint, value_type& x)
        { return emplace_hint(hint, std::move(x)); }

        template <typename InputIterator>
        void insert(InputIterator first, InputIterator last)
        {
            for (; first != last; ++first)
                insert(*first);
        }


        // void insert( std::initializer_list<value_type> ilist );
        // insert_return_type insert( tree_node&& nh );
        // iterator insert( const_iterator hint, tree_node&& nh );

        template <typename... Args>
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            using namespace leviathan::collections;
            if constexpr (detail::emplace_helper<value_type, Args...>::value)
            {
                return insert_unique((Args&&) args...);
            }
            else
            {
                return emplace_unique((Args&&) args...);
            }
        }

        // FIXME:
        template <typename... Args>
        iterator emplace_hint(const_iterator, Args&&... args)
        { return emplace((Args&&) args...).first; }

        void clear()
        { reset(); }

        iterator erase(const_iterator pos) 
        {
            auto ret = std::next(pos);
            erase_by_node(pos.const_cast_to_iterator());
            return ret.const_cast_to_iterator();
        }

        iterator erase(iterator pos)
        {
            auto ret = std::next(pos);
            erase_by_node(pos.m_ptr);
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

        size_type erase(const key_type& x)
        { return erase_by_key(x); }

        // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2077r3.html
        template <typename K> requires (IsTransparent)
        size_type erase(K&& x) 
        { return erase_by_key(x); }

        // void swap(set &other) noexcept(/* see below */);

        // tree_node extract(const_iterator position);
        // template <class K>
        // tree_node extract(K &&x);

        // void merge(); We don't want to implement this

        // Lookup
        template <typename K = key_type>
        size_type count(const key_arg_t<K>& x)
        {
            auto [lower, upper] = equal_range(x);
            return std::distance(lower, upper);
        }

        template <typename K = key_type>
        iterator find(const key_arg_t<K>& x)
        { return find_impl(x); }

        template <typename K = key_type>
        const_iterator find(const key_arg_t<K>& x) const
        { return const_cast<avl_tree&>(*this).find(x); }

        template <typename K = key_type>
        iterator lower_bound(const key_arg_t<K>& x)
        { return lower_bound_impl(x); }

        template <typename K = key_type>
        const_iterator lower_bound(const key_arg_t<K>& x) const
        { return const_cast<avl_tree&>(*this).lower_bound(x); }

        template <typename K = key_type>
        iterator upper_bound(const key_arg_t<K>& x)
        { return upper_bound_impl(x); }
        
        template <typename K = key_type>
        const_iterator upper_bound(const key_arg_t<K>& x) const
        { return const_cast<avl_tree&>(*this).upper_bound(x); }

        template <typename K = key_type>
        bool contains(const key_arg_t<K>& x) const
        { return find(x) != end(); }

        template <typename K = key_type>
        std::pair<iterator, iterator> equal_range(const key_arg_t<K>& x)
        { return equal_range_impl(x); }

        template <typename K = key_type>
        std::pair<const_iterator , const_iterator> equal_range(const key_arg_t<K>& x) const
        { return const_cast<avl_tree&>(*this).equal_range(x); }

        friend auto operator<=>(const avl_tree& lhs, const avl_tree& rhs)
        {
            return std::lexicographical_compare_three_way(
                lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        tree_node* root() 
        { return static_cast<tree_node*>(m_header.m_parent); }

        const tree_node* root() const
        { return static_cast<const tree_node*>(m_header.m_parent); }

    protected:

        using link_type = avl_node*;
        using const_link_type = const avl_node*;
        using base_ptr = avl_node_base*;
        using const_base_ptr = const avl_node_base*;


        template <typename K>
        iterator lower_bound_impl(const K& k)
        {
            base_ptr y = &m_header, x = m_header.m_parent;
            while (x)
                if (!m_cmp(keys(x), k))
                    y = x, x = x->m_left;
                else
                    x = x->m_right;
            return { y };
        }

        template <typename K>
        iterator upper_bound_impl(const K& k)
        {
            base_ptr y = &m_header, x = m_header.m_parent;
            while (x != 0)
                if (m_cmp(k, keys(x)))
                    y = x, x = x->m_left;
                else
                    x = x->m_right;
            return { y };
        }

        template <typename K>
        iterator find_impl(const K& k)
        {
            iterator j = lower_bound_impl(k);
            return (j == end() || m_cmp(k, KeyOfValue()(*j))) ? end() : j;
        }

        template <typename K>
        std::pair<iterator, iterator> equal_range_impl(const K& x)
        {
            auto lower = lower_bound_impl(x);
            auto upper = (lower == end() || m_cmp(x, KeyOfValue()(*lower))) ? lower : std::next(lower); 
            return { lower, upper };
        }

        template <typename K>
        size_type erase_by_key(const K& x)
        {
            iterator node = find_impl(x);
            if (node != end())
            {
                erase_by_node(node.m_ptr);
                return 1;
            }
            return 0;
        }

        void erase_by_node(base_ptr x)
        {      
            auto y = avl_node_base::avl_tree_rebalance_for_erase(x, m_header);
            destroy_node(static_cast<link_type>(y));
            m_size--;
        }

        link_type get_node()
        { return node_alloc_traits::allocate(m_alloc, 1); }

        void put_node(link_type p)
        { node_alloc_traits::deallocate(m_alloc, p, 1); }

        template <typename... Args>
        void construct_node(link_type node, Args&&... args)
        {
            try
            {
                node->m_parent = node->m_left = node->m_right = nullptr;
                node->m_height = 1;
                node_alloc_traits::construct(m_alloc, node->value_ptr(), (Args&&) args...);
            }
            catch (...)
            {
                // Construct or destroy may need rebind_alloc.
                // But some allocator just call std::construct_at or std::destroy_at
                // it can be compiled.
                node_alloc_traits::destroy(m_alloc, node->value_ptr());
                put_node(node);
                throw;
            }
        }

        template <typename... Args>
        link_type create_node(Args&&... args)
        {
            link_type tmp = get_node();
            construct_node(tmp, (Args&&) args...);
            return tmp;
        }

        void destroy_node(link_type p)
        { node_alloc_traits::destroy(m_alloc, p->value_ptr()); }

        void drop_node(link_type p)
        {
            destroy_node(p);
            put_node(p);
        }

        static const key_type& keys(const_link_type x)
        { return KeyOfValue()(*x->value_ptr()); }

        static const key_type& keys(const_base_ptr x)
        { return keys(static_cast<const_link_type>(x)); }

        std::pair<base_ptr, base_ptr>   
        get_insert_unique_pos(const key_type& k)
        {
            base_ptr y = &m_header, x = m_header.m_parent;
            bool comp = true;
            while (x)
            {
                y = x;
                comp = m_cmp(k, keys(x));
                x = comp ? x->m_left : x->m_right;
            }
            iterator j { y };
            if (comp)
            {
                if (j == begin())
                    return { x, y };
                else
                    --j;
            }
            if (m_cmp(keys(j.m_ptr), k))
                return { x, y };
            return { j.m_ptr, nullptr };
        }

        template <typename Arg>
        std::pair<iterator, bool> insert_unique(Arg&& v)
        {
            auto [x, p] = get_insert_unique_pos(KeyOfValue()(v));
            if (p)
            {
                return { insert_value(x, p, (Arg&&) v), true };
            }
            return { x, false };
        }

        template<typename Arg>
        iterator insert_value(base_ptr x, base_ptr p, Arg &&v)
        {
            bool insert_left = (x != 0 || p == &m_header 
                || m_cmp(KeyOfValue()(v), keys(p)));

            link_type z = create_node((Arg&&) v);

            avl_node_base::avl_tree_insert_and_rebalance(insert_left, z, p, m_header);
            ++m_size;
            return iterator(z);
        }

        iterator insert_node(base_ptr x, base_ptr p, link_type z)
        {
            bool insert_left = (x != 0 || p == &m_header || m_cmp(keys(z), keys(p)));
            avl_node_base::avl_tree_insert_and_rebalance(insert_left, z, p, m_header);
            ++m_size;
            return iterator(z);
        }

        template <typename... Args>
        std::pair<iterator, bool>
        emplace_unique(Args&& ...args)
        {
            link_type z = create_node((Args&&) args...);
            auto [x, p] = get_insert_unique_pos(keys(z));
            if (p) 
                return { insert_node(x, p, z), true };
            drop_node(z);
            return { x, false };
        }
    
        void dfs_deconstruct(base_ptr p)
        {
            if (p)
            {
                dfs_deconstruct(p->m_left);
                dfs_deconstruct(p->m_right);
                link_type pp = static_cast<link_type>(p);
                drop_node(pp);
            }
        }

        void reset()
        {
            dfs_deconstruct(m_header.m_parent);
            m_header.reset();
            m_size = 0;
        }

        avl_node_base m_header;
        size_type m_size;
        [[no_unique_address]] Compare m_cmp;
        [[no_unique_address]] node_allocator m_alloc;

    };

    template <typename T, typename Compare = std::less<>, typename Allocator = std::allocator<T>>
    class avl_set : public avl_tree<T, Compare, Allocator, identity, true> { };

    template <typename T, typename Compare = std::less<>>
    class pmr_avl_set : public avl_tree<T, Compare, std::pmr::polymorphic_allocator<T>, identity, true> { };

    template <typename K, typename V, typename Compare, typename Allocator>
    class avl_map_base : public avl_tree<std::pair<const K, V>, Compare, Allocator, select1st, true>
    {
    public:
        using mapped_type = V;
        using typename avl_tree<std::pair<const K, V>, Compare, Allocator, select1st, true>::value_type;

        struct value_compare
        {
            bool operator()(const value_type& lhs, const value_type& rhs) const
            { return m_c(lhs.first, rhs.first); }

        protected:
            value_compare(Compare c) : m_c{ c } { }
            Compare m_c;
        };

        value_compare value_comp() const
        { return value_comp(this->m_cmp); }

        // FIXME
        V& operator[](const K& key)
        { return this->insert(std::make_pair(key, V())).first->second; }

        V& operator[](K&& key)
        { return this->insert(std::make_pair(std::move(key), V())).first->second; }

    };

    template <typename K, typename V, typename Compare = std::less<>, typename Allocator = std::allocator<std::pair<const K, V>>>
    class avl_map : public avl_map_base<K, V, Compare, Allocator> { };

    template <typename K, typename V, typename Compare = std::less<>>
    class pmr_avl_map : public avl_map_base<K, V, Compare, std::pmr::polymorphic_allocator<std::pair<const K, V>>> { };

}





