#pragma once

#include "../common.hpp"
#include "../associative_container_interface.hpp"
#include "tree_drawer.hpp"

namespace leviathan::collections
{ 

namespace detail
{

template <typename Node>
concept node = requires (Node* n, const Node* cn, bool insert_left, Node& header)
{
    // Clone flag from cn.
    n->clone(cn); 

    // Call when construct an empty tree.
    n->as_empty_tree_header(); 

    // Call after allocating memory.
    n->init(); 

    // Check n is header, a helper method for iterator.
    n->is_header();

    //  Call after insertion, the param n is the father
    n->insert_and_rebalance(insert_left, n, header); 

    // Call after removing node.  
    n->rebalance_for_erase(header); 
};

} // namespace detail

/**
 * @brief Template tree for binary search tree extension.
 * 
 * This template only provide some searching methods. The actual insertion, deletion
 * and balancing operations are done by node.
 * 
 * We use NodeType as our header type which contains at least three point link
 * parent, left child and right child separately. The parent of header always
 * link the root of tree, the left child always link to the leftmost and the right
 * child always link to the rightmost. For empty tree, the left child and right
 * child of header link to itself and parent is null.
 * 
 * @param KeyValue Extractor for key and value. identity<T> for set and select1st<K, V> for map.
 * @param Compare
 * @param Allocator
 * @param UniqueKey True for set/map and False for multiset/multimap.
 * @param NodeType 
*/
template <typename KeyValue,
    typename Compare,
    typename Allocator,
    bool UniqueKey, typename NodeType>
class tree : public row_drawer, 
             public reversible_container_interface, 
             public associative_container_insertion_interface,
             public associative_container_lookup_interface<UniqueKey>
{
public:

    using key_value = KeyValue;
    using value_type = typename KeyValue::value_type;
    using key_type = typename KeyValue::key_type;
    using size_type = std::size_t;
    using allocator_type = Allocator;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using value_compare = Compare;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;

    using base_node = NodeType;
    struct tree_node : base_node
    {
        value_type m_val;

        value_type* value_ptr()
        {
            return std::addressof(m_val);
        }

        const value_type* value_ptr() const
        {
            return std::addressof(m_val);
        }

        NodeType* base()
        {
            return static_cast<NodeType*>(this);
        }

        const NodeType* base() const
        {
            return static_cast<const NodeType*>(this);
        }
    };

protected:

    using node_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<tree_node>;
    using node_alloc_traits = std::allocator_traits<node_allocator>;

    static constexpr bool IsTransparent = detail::transparent<Compare>;

    template <typename U> using key_arg_t = detail::key_arg<IsTransparent, U, key_type>;

    static constexpr bool IsNothrowMoveConstruct =
        std::is_nothrow_move_constructible_v<Compare>
        && typename node_alloc_traits::is_always_equal();

    static constexpr bool IsNothrowMoveAssign =
        std::is_nothrow_move_assignable_v<Compare>
        && typename node_alloc_traits::is_always_equal();

    static constexpr bool IsNothrowSwap =
        std::is_nothrow_swappable_v<Compare>
        && typename node_alloc_traits::is_always_equal();

    /**
     * In C++, we should not dereference a end iterator. But what about increment?
     * We hope whenever increment a iterator, the program will not abort. We make
     * iterator cycle. For instance, the final iterator incrementing will wall to
     * the sentinel(end iterator, header in our tree) and next incrementing will walk 
     * to the first element which contained by begin iterator.
     * 
     * In our implementation of `increment`, when the iterator walk to the last element,
     * the next step will always stay in place and for an empty tree, increment iterator
     * will cause infinity loop. 
     *     E.g.
     *         
     *     std::set<int>().begin()++; // infinity loop
     *     
     *     std::set<int> s{ 1, 2 };
     *     auto it = s.find(2);
     *     it++ == s.end();  // false
     *     it++ == s.end();  // true
     *     it++ == s.end();  // false
     *     it++ == s.end();  // true
     * 
     * To avoid above cases, we introduce `is_header` to help us check whether current 
     * iterator is sentinel.
    */
    struct tree_iterator : postfix_increment_and_decrement_operation, arrow_operation
    {
        using link_type = NodeType*;
        using value_type = value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        // for set, reference should be const value_type&
        // for map, reference is just value_type&
        using reference = std::conditional_t<std::is_same_v<key_type, value_type>, const value_type&, value_type&>;

        link_type m_ptr;

        constexpr tree_iterator() = default;
        constexpr tree_iterator(const tree_iterator&) = default;

        constexpr tree_iterator(link_type ptr) : m_ptr(ptr) { }

        tree_iterator left(this tree_iterator it) 
        {    
            return tree_iterator(it->m_ptr->lchild());
        }

        tree_iterator right(this tree_iterator it)
        {
            return tree_iterator(it->m_ptr->rchild());
        }

        constexpr tree_iterator& operator++()
        {
            // if node is header/end/sentinel, we simply make it cycle
            m_ptr = m_ptr->is_header() ? m_ptr->lchild() : m_ptr->increment();
            return *this;
        }

        using postfix_increment_and_decrement_operation::operator++;

        constexpr tree_iterator& operator--()
        {
            // if node is header/end/sentinel, we simply make it cycle
            m_ptr = m_ptr->is_header() ? m_ptr->rchild() : m_ptr->decrement();
            return *this;
        }

        using postfix_increment_and_decrement_operation::operator--;

        constexpr reference operator*(this tree_iterator it) 
        {
            return *(static_cast<tree_node*>(it.m_ptr)->value_ptr());
        }

        friend constexpr bool operator==(tree_iterator lhs, tree_iterator rhs) 
        {
            return lhs.m_ptr == rhs.m_ptr;
        }
    };

public:

    using iterator = tree_iterator;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:

    tree() : tree(Compare(), Allocator()) { }

    explicit tree(const Compare& compare) : tree(compare, Allocator()) { }

    explicit tree(const Allocator& alloc) : tree(Compare(), alloc) { }

    explicit tree(const Compare& compare, const Allocator& allocator)
        : m_cmp(compare), m_alloc(allocator), m_size(0)
    {
        header()->as_empty_tree_header();
    }

    tree(const tree& rhs, const Allocator& alloc) 
        : tree(rhs.m_cmp, alloc)
    {
        try
        {
            if (rhs.size())
            {
                header()->parent(clone_tree(header()->parent(), header(), rhs.header()->parent()));
                header()->lchild(header()->parent()->minimum());
                header()->rchild(header()->parent()->maximum());
                m_size = rhs.size();
            }
        }
        catch(...)
        {
            // Clear current container and rethrow exception
            clear();
            throw;  
        }
        
    }

    tree(const tree& rhs) 
        : tree(rhs, node_alloc_traits::select_on_container_copy_construction(rhs.m_alloc))
    { }

    tree(tree&& rhs) noexcept(IsNothrowMoveConstruct)
        : m_cmp(std::move(rhs.m_cmp)), 
          m_alloc(std::move(rhs.m_alloc)), 
          m_size(std::exchange(rhs.m_size, 0)), 
          m_header(rhs.m_header)
    {
        if (empty())
        {
            header->as_empty_tree_header();
        }
        rhs.header()->as_empty_tree_header();
    }    

    tree(tree&& rhs, const Allocator& alloc)
        : m_cmp(std::move(rhs.m_cmp)), m_alloc(alloc)
    {
        if (rhs.m_alloc == alloc)
        {
            m_header = rhs.header();
            rhs.header()->as_empty_tree_header();
            m_size = std::exchange(rhs.m_size, 0);
        }
        else
        {
            if (rhs.size())
            {
                header()->parent(move_tree(header()->parent(), header(), rhs.header()->parent()));
                header()->lchild(header()->parent()->minimum());
                header()->rchild(header()->parent()->maximum());
            }
            
            m_size = rhs.size();
            rhs.clear();
        }
    }

    tree& operator=(const tree& rhs)
    {
        if (std::addressof(rhs) != this)
        {
            clear();

            m_cmp = rhs.m_cmp;
            if constexpr (typename node_alloc_traits::propagate_on_container_copy_assignment())
            {
                m_alloc = rhs.m_alloc;
            }

            try
            {
                if (rhs.size())
                {
                    header()->parent(clone_tree(header()->parent(), header(), rhs.header()->parent()));
                    header()->lchild(header()->parent()->minimum());
                    header()->rchild(header()->parent()->maximum());
                    m_size = rhs.size();
                }
            }
            catch (...)
            {
                clear();
                throw;
            }
        }
        return *this;
    }

    tree& operator=(tree& rhs) noexcept(IsNothrowMoveAssign)
    {
        if (std::addressof(rhs) != this)
        {
            clear();
            m_cmp = std::move(rhs.m_cmp);
            if (typename node_alloc_traits::propagate_on_container_move_assignment())
            {
                m_alloc = std::move(rhs.m_alloc);
                m_header = rhs.m_header;
                rhs.header()->as_empty_tree_header();
            }
            else if (m_alloc == rhs.m_alloc) 
            {
                m_header = rhs.m_header;
                rhs.header()->as_empty_tree_header();
            }
            else
            {
                // Exceptions may thrown
                move_from_other(std::move(rhs));
                rhs.clear();
            }
            m_size = std::exchange(rhs.m_size, 0);
        }
        return *this;
    }

    friend void swap(tree& lhs, tree& rhs) noexcept(IsNothrowSwap)
    {
        using std::swap;
        swap(lhs.m_header, rhs.m_header);
        swap(lhs.m_cmp, rhs.m_cmp);
        swap(lhs.m_size, rhs.m_size);
        if constexpr (typename node_alloc_traits::propagate_on_container_swap())
        {
            swap(lhs.m_alloc, rhs.m_alloc);
        }
    }

    ~tree()
    {
        clear();
    }

    void clear()
    {
        reset();
    }

    iterator begin()
    {
        return iterator(header()->lchild());
    }

    const_iterator begin() const
    {
        return iterator(header()->lchild());
    }

    iterator end()
    {
        return iterator(header());
    }

    const_iterator end() const
    {
        return iterator(header());
    }

    size_type size() const
    {
        return m_size;
    }

    bool empty() const
    {
        return size() == 0;
    }

    allocator_type get_allocator() const
    {
        return allocator_type(m_alloc);
    }

    size_type max_size() const
    {
        return node_alloc_traits::max_size(m_alloc);
    }

    key_compare key_comp() const
    {
        return m_cmp;
    }

    value_compare value_comp() const
    {
        return m_cmp;
    }

    // Modifiers
    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        using namespace leviathan::collections;
        if constexpr (detail::emplace_helper<value_type, Args...>::value)
        {
            return insert_unique((Args&&)args...);
        }
        else
        {
            return emplace_unique((Args&&)args...);
        }
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace_hint(const_iterator pos, Args&&... args)
    {
        return emplace((Args&&) args...);
    }

    iterator erase(const_iterator pos) 
        requires (!std::same_as<iterator, const_iterator>)
    {
        return erase(pos.base());
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
        {
            clear();
        }
        else
        {
            for (; first != last; first = erase(first));
        }
        return last;
    }

    iterator erase(const_iterator first, const_iterator last) 
        requires (!std::same_as<iterator, const_iterator>)
    { 
        return erase(first.base(), last.base()); 
    }

    size_type erase(const key_type &x)
    {
        return erase_by_key(x);
    }

    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2077r3.html
    template <typename K> requires (IsTransparent)
    size_type erase(K&& x) 
    { 
        return erase_by_key(x); 
    }

    template <typename K = key_type>
    iterator lower_bound(const key_arg_t<K> &x)
    {
        return lower_bound_impl(x);
    }

    template <typename K = key_type>
    const_iterator lower_bound(const key_arg_t<K> &x) const
    {
        return const_cast<tree &>(*this).lower_bound(x);
    }

    NodeType* header() 
    { 
        return &m_header; 
    }
    
    const NodeType* header() const 
    { 
        return &m_header; 
    }

    tree_node *root()
    {
        return static_cast<tree_node*>
            (header()->parent());
    }

    const tree_node *root() const
    {
        return static_cast<const tree_node*>
            (header()->parent());
    }

protected:

    using link_type = tree_node*;
    using const_link_type = const tree_node*;
    using base_ptr = NodeType*;
    using const_base_ptr = const NodeType*;

    template <typename K>
    iterator find_impl(const K& k)
    {
        iterator j = lower_bound_impl(k);
        return (j == end() || m_cmp(k, KeyValue()(*j))) ? end() : j;
    }

    void erase_by_node(base_ptr x)
    {      
        // auto y = NodeType::rebalance_for_erase(x, m_header);
        auto y = x->rebalance_for_erase(m_header);
        destroy_node(static_cast<link_type>(y));
        m_size--;
    }

    template <typename K>
    size_type erase_by_key(const K& x)
    {
        iterator node = this->find_impl(x);
        if (node != end())
        {
            erase_by_node(node.m_ptr);
            return 1;
        }
        return 0;
    }

    template <typename K>
    iterator lower_bound_impl(const K& k)
    {
        base_ptr y = &m_header, x = header()->parent();
        while (x)
            if (!m_cmp(keys(x), k))
            {
                // y = x, x = NodeType::left(x);
                y = x, x = x->lchild();
            }
            else
            {
                // x = NodeType::right(x);
                x = x->rchild();
            }
        return { y };
    }

    void dfs_deconstruct(base_ptr p)
    {
        if (p)
        {
            dfs_deconstruct(p->lchild());
            dfs_deconstruct(p->rchild());
            link_type node = static_cast<link_type>(p);
            drop_node(node);
        }
    }

    void reset()
    {
        dfs_deconstruct(header()->parent());
        header()->as_empty_tree_header();
        m_size = 0;
    }

    template <typename Arg>
    std::pair<iterator, bool> insert_unique(Arg&& v)
    {
        auto [x, p] = get_insert_unique_pos(KeyValue()(v));
        if (p)
        {
            return { insert_value(x, p, (Arg&&)v), true };
        }
        return { x, false };
    }

    template <typename... Args>
    std::pair<iterator, bool>
        emplace_unique(Args&& ...args)
    {
        link_type z = create_node((Args&&)args...);
        auto [x, p] = get_insert_unique_pos(keys(z));
        if (p)
            return { insert_node(x, p, z), true };
        drop_node(z);
        return { x, false };
    }

    iterator insert_node(base_ptr x, base_ptr p, link_type z)
    {
        bool insert_left = (x != 0 || p == &m_header || m_cmp(keys(z), keys(p)));
        //NodeType::insert_and_rebalance(insert_left, z, p, m_header);
        z->insert_and_rebalance(insert_left, p, m_header);
        ++m_size;
        return iterator(z);
    }

    template<typename Arg>
    iterator insert_value(base_ptr x, base_ptr p, Arg&& v)
    {
        bool insert_left = (x != 0 || p == &m_header
            || m_cmp(KeyValue()(v), keys(p)));

        link_type z = create_node((Arg&&)v);

        //NodeType::insert_and_rebalance(insert_left, z, p, m_header);
        z->insert_and_rebalance(insert_left, p, m_header);
        ++m_size;
        return iterator(z);
    }

    std::pair<base_ptr, base_ptr> 
        get_insert_unique_pos(const key_type& k)
    {
        base_ptr y = &m_header, x = header()->parent();
        bool comp = true;

        while (x)
        {
            y = x;
            comp = m_cmp(k, keys(x));
            x = comp ? x->lchild() : x->rchild();
        }

        iterator j{ y };

        if (comp)
        {
            if (j == begin())
            {
                return { x, y };
            }
            else
            {
                --j;
            }
        }

        if (m_cmp(KeyValue()(*j), k))
        {
            return { x, y };
        }

        return { j.m_ptr, nullptr };
    }

    static const key_type& keys(const_link_type x)
    {
        return KeyValue()(*x->value_ptr());
    }

    static const key_type& keys(const_base_ptr x)
    {
        return keys(static_cast<const_link_type>(x));
    }

    link_type get_node()
    {
        return node_alloc_traits::allocate(m_alloc, 1);
    }

    void put_node(link_type p)
    {
        node_alloc_traits::deallocate(m_alloc, p, 1);
    }

    template <typename... Args>
    void construct_node(link_type node, Args&&... args)
    {
        try
        {
            //NodeType::init(node);
            node->init();
            node_alloc_traits::construct(m_alloc, node->value_ptr(), (Args&&)args...);
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
        construct_node(tmp, (Args&&)args...);
        return tmp;
    }

    void destroy_node(link_type p)
    {
        node_alloc_traits::destroy(m_alloc, p->value_ptr());
    }

    void drop_node(link_type p)
    {
        destroy_node(p);
        put_node(p);
    }

    /**
     * @brief Deepcopy tree
     * 
     * @param x: Current node
     * @param p: Parent of current node
     * @param y: Copied node
    */
    base_node* clone_tree(base_node* x, base_node* p, const base_node* y)
    {
        if (!y)
        {
            return nullptr;
        }

        x = create_node(*static_cast<const tree_node*>(y)->value_ptr());
        x->clone(y);
        x->lchild(clone_tree(x->lchild(), x, y->lchild()));
        x->rchild(clone_tree(x->rchild(), x, y->rchild()));
        x->parent(p);
        return x;
    }

    /**
     * @brief Move the elements from y to x
     * 
     * @param x: Current node
     * @param p: Parent of current node
     * @param y: Moved node
    */
    base_node* move_tree(base_node* x, base_node* p, const base_node* y)
    {
        if (!y)
        {
            return nullptr;
        }

        x = create_node(
            std::move_if_noexcept(
                *static_cast<tree_node*>(y)->value_ptr()
            )
        );

        x->clone(y);
        x->lchild(clone_tree(x->lchild(), x, y->lchild()));
        x->rchild(clone_tree(x->rchild(), x, y->rchild()));
        x->parent(p);
        return x;
    }

    [[no_unique_address]] Compare m_cmp;
    [[no_unique_address]] node_allocator m_alloc;

    /**
     * We use a NodeType which does not have value_type field as header node.
     * The parent of node link to root of tree, the left child link to the leftmost of tree
     * and the right child link to the rightmost node. For `begin`, we return the leftmost node
     * of tree and for `end`, we return the header as sentinel. For empty tree, the parent
     * of m_header link nullptr, the left and right link to itself. 
    */
    NodeType m_header;

    /* Size of current tree */
    size_type m_size;
};

template <typename T, typename Compare, typename Allocator, bool Unique, typename NodeType>
class tree_set : public tree<identity<T>, Compare, Allocator, Unique, NodeType>
{
    using base = tree<identity<T>, Compare, Allocator, Unique, NodeType>;
    using base::base;
    using base::operator=;
};

template <typename K, typename V, typename Compare, typename Allocator, bool Unique, typename NodeType>
class tree_map : public tree<select1st<K, V>, Compare, Allocator, Unique, NodeType>
{
    using base = tree<select1st<K, V>, Compare, Allocator, Unique, NodeType>;

public:

    using mapped_type = V;
    using typename base::value_type;
    using typename base::iterator;
    using typename base::const_iterator;

    struct value_compare : ordered_map_container_value_compare<value_type, Compare>
    {
    protected:
        friend class tree_map;
        
        value_compare(Compare compare) 
            : ordered_map_container_value_compare<value_type, Compare>(compare) { }
    };

    value_compare value_comp() const
    {
        return value_compare(this->m_cmp);
    }

    V &operator[](const K &key)
    {
        return this->try_emplace(key).first->second;
    }

    V &operator[](K &&key)
    {
        return this->try_emplace(std::move(key)).first->second;
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const K &k, Args &&...args)
    {
        return try_emplace_impl(k, (Args &&)args...);
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(K &&k, Args &&...args)
    {
        return try_emplace_impl(std::move(k), (Args &&)args...);
    }

    // FIXME
    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const_iterator, const K &k, Args &&...args)
    {
        return try_emplace_impl(k, (Args &&)args...);
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const_iterator, K &&k, Args &&...args)
    {
        return try_emplace_impl(std::move(k), (Args &&)args...);
    }

    template <typename M>
    std::pair<iterator, bool> insert_or_assign(const K &k, M &&obj)
    {
        return insert_or_assign_impl(k, (M &&)obj);
    }

    template <typename M>
    std::pair<iterator, bool> insert_or_assign(K &&k, M &&obj)
    {
        return insert_or_assign_impl(std::move(k), (M &&)obj);
    }

protected:

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