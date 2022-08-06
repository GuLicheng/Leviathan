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

    template <typename T, typename BaseNode>
    struct tree_node_impl : public BaseNode
    {
        T m_val;
        // alignas(T) unsigned char m_val[sizeof(T)];

        T* value_ptr() 
        { return reinterpret_cast<T*>(std::addressof(m_val)); }

        const T* value_ptr() const
        { return reinterpret_cast<const T*>(std::addressof(m_val)); }

        BaseNode* base() 
        { return static_cast<BaseNode*>(this); }

        const BaseNode* base() const
        { return static_cast<const BaseNode*>(this); }

    };

    template <typename T, typename Compare, typename Allocator, typename KeyOfValue, bool UniqueKey, tree_node_interface NodeType>
    class tree
    {
        static_assert(UniqueKey, "Not Support MultiKey");

    public:

        // for visual
        using tree_node_base = NodeType;
        using tree_node = tree_node_impl<T, NodeType>;

    private:

        using node_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<tree_node>;
        using node_alloc_traits = typename std::allocator_traits<node_allocator>;

        constexpr static bool IsTransparent = leviathan::collections::detail::is_transparent<Compare>;
        
        using Key = typename KeyOfValue::template type<T>;

        template <typename U> using key_arg_t = leviathan::collections::detail::key_arg<IsTransparent, U, Key>;

        template <bool Const>
        struct tree_iterator
        {
            using link_type = std::conditional_t<Const, const NodeType*, NodeType*>;
            using value_type = std::conditional_t<Const, const T, T>;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::bidirectional_iterator_tag;
            // for set, reference should be const value_type&
            // for map, reference is just value_type&
            using reference = std::conditional_t<std::is_same_v<Key, T>, const value_type&, value_type&>;

            link_type m_ptr;

            constexpr tree_iterator() noexcept = default;
            constexpr tree_iterator(const tree_iterator&) noexcept = default;

            constexpr tree_iterator(const tree_iterator<!Const>& rhs) noexcept requires (Const)
                : m_ptr{ rhs.m_ptr } { }

            constexpr tree_iterator(link_type ptr) noexcept
                : m_ptr{ ptr } { }

            constexpr bool is_header() const 
            { return NodeType::is_header(m_ptr); }

            constexpr tree_iterator& operator++()
            {
                // if node is header/end/sentinel, we simply make it cycle
                m_ptr = is_header() ? NodeType::left(m_ptr) : NodeType::increment(m_ptr);
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
                m_ptr = is_header() ? NodeType::right(m_ptr) : NodeType::decrement(m_ptr);
                return *this;
            }

            constexpr tree_iterator operator--(int)
            {
                auto old = *this;
                --*this;
                return old;
            } 

            constexpr reference operator*() const 
            {
                using cast_link_type = std::conditional_t<Const, const tree_node*, tree_node*>;  
                return *(static_cast<cast_link_type>(m_ptr)->value_ptr()); 
            }

            constexpr auto operator->() const
            { return std::addressof(this->operator*()); }
            
            constexpr bool operator==(const tree_iterator&) const = default;

            tree_iterator<!Const> const_cast_to_iterator() const requires (Const)
            { return tree_iterator<!Const>(const_cast<typename tree_iterator<!Const>::link_type>(m_ptr)); }

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

    public:

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


        tree() : tree(Compare(), Allocator()) { }

        explicit tree(const Compare& compare, const Allocator& allocator = Allocator())
            : m_alloc{ allocator }, m_cmp{ compare }, m_size{ 0 }
        {
            NodeType::reset(header());
        }

        explicit tree(const Allocator& alloc)
            : tree(Compare(), alloc) { }


        ~tree()
        { clear(); }

        void copy_from_other(const tree& rhs)
        {
            if (!rhs.root())
                return;
            // header()->m_parent = clone_tree(header()->m_parent, header(), rhs.m_header.m_parent);
            // header()->m_left = NodeType::minimum(m_header.m_parent);
            // header()->m_right = NodeType::maximum(m_header.m_parent);
            NodeType::set_parent(header(), clone_tree(root(), header(), rhs.root()));
            NodeType::set_left(header(), NodeType::minimum(root()));
            NodeType::set_right(header(), NodeType::maximum(root()));
        }

        void move_from_other(tree&& rhs)
        {
            if (!rhs.root())
                return;
            NodeType::set_parent(header(), move_tree(root(), header(), rhs.root()));
            NodeType::set_left(header(), NodeType::minimum(root()));
            NodeType::set_right(header(), NodeType::maximum(root()));
        }

        tree(const tree& rhs) 
            : m_alloc{ node_alloc_traits::select_on_container_copy_construction(rhs.m_alloc) },
              m_size{ rhs.m_size },
              m_cmp{ rhs.m_cmp }
        {
			try 
			{
                copy_from_other(rhs);
			}
			catch (...)
			{
				clear();
				throw; // rethrow exception
			}
		}

        tree(tree&& rhs) noexcept(IsNothrowMoveConstruct)
			: m_cmp{ std::move(rhs.m_cmp) }, m_alloc{ std::move(rhs.m_alloc) }, m_size{ rhs.m_size }, m_header{ rhs.m_header }
		{
            if (!rhs.root())
                NodeType::reset(header());
            NodeType::reset(rhs.header());
		}        


        tree& operator=(const tree& rhs)
        {
            if (std::addressof(rhs) != this)
			{
                clear();
                // copy member
                m_size = rhs.m_size;
                m_cmp = rhs.m_cmp;
				if constexpr (typename node_alloc_traits::propagate_on_container_copy_assignment())
					m_alloc = rhs.m_alloc;
				try
				{
                    copy_from_other(rhs);
				}
				catch (...)
				{
					clear();
					throw;
				}
			}
			return *this;
        }

        tree& operator=(tree&& rhs) noexcept(IsNothrowMoveAssign)
        {
            if (this != std::addressof(rhs))
			{
                clear();
                m_cmp = std::move(rhs.m_cmp);
                m_size = rhs.m_size;
                if (rhs.root())
                    m_header = rhs.m_header;
				if constexpr (typename node_alloc_traits::propagate_on_container_move_assignment())
				{
					m_alloc = std::move(rhs.m_alloc);
				}
				else
				{
                    move_from_other(std::move(rhs));
				}
                NodeType::reset(rhs.header());
			}
			return *this;
        }

        void swap(tree &rhs) noexcept(IsNothrowSwap)
        {
			if (this != std::addressof(rhs))
			{
				if constexpr (typename node_alloc_traits::propagate_on_container_swap())
				{
					swap_impl(rhs);
					std::swap(m_alloc, rhs.m_alloc);
				}
				else if (typename node_alloc_traits::is_always_equal() || m_alloc == rhs.m_alloc)
				{
					swap_impl(rhs);
				}
				else
				{
                    assert(false && "Undefined Behaviour");
				}
			}
        }

        // Iterators
        iterator begin() 
        { return { NodeType::left(header()) }; }

        const_iterator begin() const 
        { return { NodeType::left(header()) }; }

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


        // node_type extract(const_iterator position);
        // template <class K>
        // node_type extract(K &&x);

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
        { return const_cast<tree&>(*this).find(x); }

        template <typename K = key_type>
        iterator lower_bound(const key_arg_t<K>& x)
        { return lower_bound_impl(x); }

        template <typename K = key_type>
        const_iterator lower_bound(const key_arg_t<K>& x) const
        { return const_cast<tree&>(*this).lower_bound(x); }

        template <typename K = key_type>
        iterator upper_bound(const key_arg_t<K>& x)
        { return upper_bound_impl(x); }
        
        template <typename K = key_type>
        const_iterator upper_bound(const key_arg_t<K>& x) const
        { return const_cast<tree&>(*this).upper_bound(x); }

        template <typename K = key_type>
        bool contains(const key_arg_t<K>& x) const
        { return find(x) != end(); }

        template <typename K = key_type>
        std::pair<iterator, iterator> equal_range(const key_arg_t<K>& x)
        { return equal_range_impl(x); }

        template <typename K = key_type>
        std::pair<const_iterator, const_iterator> equal_range(const key_arg_t<K>& x) const
        { return const_cast<tree&>(*this).equal_range(x); }

        friend auto operator<=>(const tree& lhs, const tree& rhs)
        {
            return std::lexicographical_compare_three_way(
                lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        tree_node* root() 
        { return static_cast<tree_node*>(NodeType::parent(header())); }

        const tree_node* root() const
        { return static_cast<const tree_node*>(NodeType::parent(header())); }

        NodeType* header() 
        { return std::addressof(m_header); }

        const NodeType* header() const
        { return std::addressof(m_header); }

    protected:

        using link_type = tree_node*;
        using const_link_type = const tree_node*;
        using base_ptr = NodeType*;
        using const_base_ptr = const NodeType*;

        void swap_impl(tree& rhs)
        {
            using std::swap;
            swap(m_size, rhs.m_size);
            swap(m_cmp, rhs.m_cmp);
            swap(m_header, rhs.m_header);
        }

        base_ptr clone_tree(tree_node_base* cur, tree_node_base* parent, const tree_node_base* cloned_node)
        {
            if (!cloned_node)
                return nullptr;

            cur = create_node(*(static_cast<const tree_node*>(cloned_node)->value_ptr()));
            NodeType::clone(cur, cloned_node);
            NodeType::set_left(cur, clone_tree(NodeType::left(cur), cur, NodeType::left(cloned_node)));
            NodeType::set_right(cur, clone_tree(NodeType::right(cur), cur, NodeType::right(cloned_node)));
            NodeType::set_parent(cur, parent);
            return cur;                
        }

        base_ptr move_tree(tree_node_base* cur, tree_node_base* parent, tree_node_base* cloned_node)
        {
            // throw std::runtime_error("Your allocator is very bad!");
            if (!cloned_node)
                return nullptr;

            cur = create_node(
                std::move_if_noexcept(*(static_cast<const tree_node*>(cloned_node)->value_ptr())) // only difference between clone_tree
            );
            NodeType::clone(cur, cloned_node);
            NodeType::set_left(cur, clone_tree(NodeType::left(cur), cur, NodeType::left(cloned_node)));
            NodeType::set_right(cur, clone_tree(NodeType::right(cur), cur, NodeType::right(cloned_node)));
            NodeType::set_parent(cur, parent);
            return cur;                
        }

        template <typename K>
        iterator lower_bound_impl(const K& k)
        {
            base_ptr y = &m_header, x = NodeType::parent(header());
            while (x)
                if (!m_cmp(keys(x), k))
                    y = x, x = NodeType::left(x);
                else
                    x = NodeType::right(x);
            return { y };
        }

        template <typename K>
        iterator upper_bound_impl(const K& k)
        {
            base_ptr y = &m_header, x = NodeType::parent(header());
            while (x)
                if (m_cmp(k, keys(x)))
                    y = x, x = NodeType::left(x);
                else
                    x = NodeType::right(x);
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
            auto y = NodeType::rebalance_for_erase(x, m_header);
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
                NodeType::init(node);
                node_alloc_traits::construct(m_alloc, node->value_ptr(), (Args&&) args...);
            }
            catch (...)
            {
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
            base_ptr y = &m_header, x = NodeType::parent(header());
            bool comp = true;
            while (x)
            {
                y = x;
                comp = m_cmp(k, keys(x));
                x = comp ? NodeType::left(x) : NodeType::right(x);
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
        iterator insert_value(base_ptr x, base_ptr p, Arg&& v)
        {
            bool insert_left = (x != 0 || p == &m_header 
                || m_cmp(KeyOfValue()(v), keys(p)));

            link_type z = create_node((Arg&&) v);

            NodeType::insert_and_rebalance(insert_left, z, p, m_header);
            ++m_size;
            return iterator(z);
        }

        iterator insert_node(base_ptr x, base_ptr p, link_type z)
        {
            bool insert_left = (x != 0 || p == &m_header || m_cmp(keys(z), keys(p)));
            NodeType::insert_and_rebalance(insert_left, z, p, m_header);
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
                dfs_deconstruct(NodeType::left(p));
                dfs_deconstruct(NodeType::right(p));
                link_type pp = static_cast<link_type>(p);
                drop_node(pp);
            }
        }

        void reset()
        {
            dfs_deconstruct(root());
            NodeType::reset(header());
            m_size = 0;
        }

        NodeType m_header;
        size_type m_size;
        [[no_unique_address]] Compare m_cmp;
        [[no_unique_address]] node_allocator m_alloc;

    };

    template <typename T, typename Compare, typename Allocator, typename NodeType>
    class tree_set : public tree<T, Compare, Allocator, identity, true, NodeType> { };

    template <typename K, typename V, typename Compare, typename Allocator, typename NodeType>
    class tree_map : public tree<std::pair<const K, V>, Compare, Allocator, select1st, true, NodeType>
    {
        using base_tree_type = tree<std::pair<const K, V>, Compare, Allocator, select1st, true, NodeType>;
    public:
        using mapped_type = V;
        using typename base_tree_type::value_type;
        using typename base_tree_type::iterator;
        using typename base_tree_type::const_iterator;

        struct value_compare
        {
            bool operator()(const value_type& lhs, const value_type& rhs) const
            { return m_c(lhs.first, rhs.first); }

        protected:
            friend class tree_map;
            value_compare(Compare c) : m_c{ c } { }
            Compare m_c;
        };

        value_compare value_comp() const
        { return value_compare(this->m_cmp); }

        V& operator[](const K& key)
        { return this->try_emplace(key).first->second; }

        V& operator[](K&& key)
        { return this->try_emplace(std::move(key)).first->second; }

        template <typename... Args>
        std::pair<iterator, bool> try_emplace(const K& k, Args&&... args)
        { return try_emplace_impl(k, (Args&&) args...); }

        template <typename... Args>
        std::pair<iterator, bool> try_emplace(K&& k, Args&&... args)
        { return try_emplace_impl(std::move(k), (Args&&) args...); }

        // FIXME
        template <typename... Args>
        std::pair<iterator, bool> try_emplace(const_iterator, const K& k, Args&&... args)
        { return try_emplace_impl(k, (Args&&) args...); }

        template <typename... Args>
        std::pair<iterator, bool> try_emplace(const_iterator, K&& k, Args&&... args)
        { return try_emplace_impl(std::move(k), (Args&&) args...); }

        template <typename M>
        std::pair<iterator, bool> insert_or_assign(const K& k, M&& obj)
        { return insert_or_assign_impl(k, (M&&)obj); }

        template <typename M>
        std::pair<iterator, bool> insert_or_assign(K&& k, M&& obj)
        { return insert_or_assign_impl(std::move(k), (M&&)obj); }


    private:

        template <typename KK, typename M>
        std::pair<iterator, bool> insert_or_assign_impl(KK&& k, M&& obj)
        {
            auto [x, p] = this->get_insert_unique_pos(k);
            if (p) 
            {
                auto z = this->create_node((KK&&)k, (M&&)obj);
                return { this->insert_node(x, p, z), true };
            }
            auto j = iterator(x);
            *j = (M&&)obj;
            return { j, false };
        }

        template <typename KK, typename... Args>
        std::pair<iterator, bool> try_emplace_impl(KK&& k, Args&&... args)
        {
            auto [x, p] = this->get_insert_unique_pos(k);
            if (p) 
            {
                auto z = this->create_node(
                    std::piecewise_construct, 
                    std::forward_as_tuple((KK&)k), 
                    std::forward_as_tuple((Args&&)args...));
                return { this->insert_node(x, p, z), true };
            }
            return { x, false };
        }


    };

}


namespace std
{
    template <typename K, typename V, typename Compare, typename Allocator, typename NodeType>
    void swap(::leviathan::collections::tree_map<K, V, Compare, Allocator, NodeType>& lhs,
              ::leviathan::collections::tree_map<K, V, Compare, Allocator, NodeType>& rhs)
    noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }

    template <typename T, typename Compare, typename Allocator, typename NodeType>
    void swap(::leviathan::collections::tree_set<T, Compare, Allocator, NodeType>& lhs,
              ::leviathan::collections::tree_set<T, Compare, Allocator, NodeType>& rhs)
    noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }
    
}


