#pragma once

#include <leviathan/collections/common.hpp>
#include <leviathan/collections/tree/tree_iterator.hpp>
#include <leviathan/collections/tree/tree_drawer.hpp>
#include <leviathan/collections/node_handle.hpp>
#include <leviathan/collections/container_interface.hpp>

namespace cpp::collections
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
 * @brief Template for extended forms of binary search tree.
 * 
 * This template only provide some searching methods. The actual insertion, deletion
 * and balancing operations are done by Node.
 * 
 * We use Node as our header type which contains at least three points link
 * parent, left child and right child separately. The actually tree node should
 * inherit from Node with value filed. The value field is used to store the actual
 * data. The Node should provide some basic operations include clone, init, 
 * as_empty_tree_header, is_header, insert_and_rebalance and rebalance_for_erase. 
 * 
 * To enable constant time begin(), to the leftmost node of the tree, and to enable 
 * linear time performance when used with generic set algorithms(set_union, etc...). 
 * For empty tree, the left child and right child of header link to itself and parent is null.
 * 
 * For delete operation, no elements are copied or moved, only the internal pointers 
 * of the container nodes are repointed.
 * 
 * For root node(if exist), its parent always link header. It means for all nodes(except header)
 * in tree, their parent will never not null, and the parent of header can be nullptr
 * if and only if the tree is empty. So our operation can be simplified since we can
 * always assume the parent is not null.
 * 
 * @param KeyOfValue Extractor extract key from value. identity<T> for set and select1st<K, V> for map
 * @param Compare Compare Key comparison function object
 * @param Allocator Allocator Type of the allocator object used to define the storage allocation model
 * @param UniqueKey True for set/map and False for multiset/multimap
 * @param Node Type of tree node with basic tree operations but value field
*/
template <typename KeyOfValue, typename Compare, typename Allocator, bool UniqueKey, typename Node>
class tree : public row_drawer, 
             public iterable_interface, 
             public std::conditional_t<UniqueKey, unique_insert_interface, insert_interface>,
             public erase_interface,
             public lookup_interface
{
    template <typename A, typename B, typename C, bool D, typename E>
    friend class tree;

    using insert_functions = std::conditional_t<UniqueKey, unique_insert_interface, insert_interface>;

public:

    using value_type = typename KeyOfValue::value_type;
    using key_type = typename KeyOfValue::key_type;
    using reference = typename KeyOfValue::reference;
    using const_reference = typename KeyOfValue::const_reference;
    using size_type = std::size_t;
    using allocator_type = Allocator;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using value_compare = Compare;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    using tree_node = value_field<Node, value_type>;

protected:

    using alloc_traits = std::allocator_traits<Allocator>;
    using node_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<tree_node>;
    using node_alloc_traits = std::allocator_traits<node_allocator>;
    using node_base = Node;

public:

    using iterator = tree_iterator<KeyOfValue, node_base, tree_node>;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using node_type = node_handle<KeyOfValue, node_allocator>;
    using insert_return_type = node_insert_return<iterator, node_type>;

protected:

    template <typename U> 
    using key_arg_t = detail::key_arg<detail::transparent<Compare>, U, key_type>;

    static constexpr bool IsNothrowMoveConstruct = nothrow_move_constructible<Allocator, Compare>;
    static constexpr bool IsNothrowMoveAssign = nothrow_move_assignable<Allocator, Compare>;
    static constexpr bool IsNothrowSwap = nothrow_swappable<Allocator, Compare>;

public:

    tree() : tree(Compare()) { }

    explicit tree(const Compare& comp, const Allocator& alloc = allocator_type())
        : m_cmp(comp), m_alloc(alloc), m_size(0)
    {
        make_header_sentinel();
    }

    explicit tree(const Allocator& alloc) : tree(Compare(), alloc) { }

    template <typename InputIt>
    tree(InputIt first, InputIt last, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
        : tree(comp, alloc)
    {
        insert(first, last);
    }

    tree(std::initializer_list<value_type> ilist, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
        : tree(ilist.begin(), ilist.end(), comp, alloc) { }

    tree(std::initializer_list<value_type> ilist, const Allocator& alloc)
        : tree(ilist, Compare(), alloc) { }

    template <container_compatible_range<value_type> R> 
    tree(std::from_range_t, R&& rg, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
        : tree(std::ranges::begin(rg), std::ranges::end(rg), comp, alloc) { }
        
    template <container_compatible_range<value_type> R> 
    tree(std::from_range_t, R&& rg, const Allocator& alloc)
        : tree(std::ranges::begin(rg), std::ranges::end(rg), Compare(), alloc) { }

    tree(const tree& other) 
        : tree(other, alloc_traits::select_on_container_copy_construction(other.m_alloc)) { }

    tree(tree&& other) noexcept(IsNothrowMoveConstruct)
        : m_cmp(std::move(other.m_cmp)), 
          m_alloc(std::move(other.m_alloc)), 
          m_size(std::exchange(other.m_size, 0))
    {
        empty() ? make_header_sentinel() : stolen_header(other);
    }

    tree(const tree& other, const Allocator& alloc) : tree(other.m_cmp, alloc)
    {
        copy_or_move_from_another_tree(other, [](const auto& x) static -> const auto& { return x; });
    }

    tree(tree&& other, const Allocator& alloc) 
        : m_cmp(std::move(other.m_cmp)), m_alloc(alloc), m_size(std::exchange(other.m_size, 0))
    {
        if (alloc == other.m_alloc)
        {
            swap(other);
        }
        else
        {
            copy_or_move_from_another_tree(other, [](auto&& x) static -> auto&& { return std::move(x); });
        }
    }

    ~tree()
    {
        clear();
    }

    tree& operator=(const tree& other)
    {
        if (this != std::addressof(other))
        {
            clear();

            m_cmp = other.m_cmp;

            if constexpr (alloc_traits::propagate_on_container_copy_assignment::value)
            {
                m_alloc = other.m_alloc;
            }

            copy_or_move_from_another_tree(other, [](const auto& x) static -> const auto& { return x; });
        }

        return *this;
    }

    tree& operator=(tree&& other) noexcept(IsNothrowMoveAssign)
    {
        if (this != std::addressof(other))
        {
            using std::swap;
            clear();

            if constexpr (alloc_traits::propagate_on_container_move_assignment::value)
            {
                swap(m_alloc, other.m_alloc);
                swap(m_cmp, other.m_cmp);
                swap(m_size, other.m_size);
                stolen_header(other);
            }
            else if (m_alloc == other.m_alloc) 
            {
                swap(m_cmp, other.m_cmp);
                swap(m_size, other.m_size);
                stolen_header(other);
            }
            else
            {
                m_cmp = other.m_cmp;
                copy_or_move_from_another_tree(other, [](auto&& x) static -> auto&& { return std::move(x); });
            }
        }

        return *this;
    }

    tree& operator=(std::initializer_list<value_type> ilist)
    {
        clear();
        insert(ilist.begin(), ilist.end());
        return *this;
    }

    // Iterators
    template <typename Self>
    self_iter_t<Self> begin(this Self&& self)
    {
        return iterator(as_non_const(self).header()->lchild());
    }

    template <typename Self>
    self_iter_t<Self> end(this Self&& self)
    {
        return iterator(as_non_const(self).header());
    }

    // Member functions
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
        return m_alloc;
    }

    size_type max_size() const
    {
        return std::allocator_traits<Allocator>::max_size(m_alloc);
    }

    static KeyOfValue key_of_value() 
    {
        return KeyOfValue();    
    }

    // Observers
    key_compare key_comp() const
    {
        return m_cmp;
    }

    value_compare value_comp() const
    {
        return m_cmp;
    }

    void clear()
    {
        reset();
    }

    // Lookup
    template <typename Self, typename K = key_type>
    self_iter_t<Self> lower_bound(this Self&& self, const key_arg_t<K>& x)
    {
        return as_non_const(self).lower_bound_impl(x);
    }

    template <typename Self, typename K = key_type>
    self_iter_t<Self> upper_bound(this Self&& self, const key_arg_t<K>& x)
    {
        return as_non_const(self).upper_bound_impl(x);
    }

    // Modifiers
    template <typename... Args>
    auto emplace(Args&&... args)
    {
        if constexpr (UniqueKey)
        {
            if constexpr (detail::emplace_helper<value_type, Args...>::value || 
                          (sizeof...(Args) == 1 && detail::transparent<Compare>))
            {
                // For value_type models std::string,
                // if arg is std::string, 
                // or arg is const char* and Compare::transparent is defined, 
                // we can use insert_unique directly.
                // 
                // Follow overloads will enter this branch:
                // insert(const value_type&);
                // insert(value_type&&);
                // template <typename K> insert(K&&);
                return insert_unique((Args&&)args...);
            }
            else
            {
                // We use a value_handle to help us destroy, we construct value at stack since
                // the tree may contains the element with equivalent key. If key conflict,
                // we can avoid the operation of memory allocation and deallocation. We 
                // assume the cost of move operation is cheaper. 
                value_handle<value_type, allocator_type> handle(m_alloc, (Args&&)args...);
                return insert_unique(*handle);
            }
        }
        else
        {
            auto node = create_node((Args&&) args...);
            auto [p, insert_left] = get_insert_pos(keys(node));
            return insert_node(insert_left, p, node);
        }
    }

    using erase_interface::erase;

    size_type erase(const key_type& key)
    {
        return erase_by_key(key);
    }

    const_iterator erase(const_iterator pos)
    {
        auto ret = std::next(pos);
        erase_by_node(pos.base().link());
        return ret;
    }

    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2077r3.html
    template <typename KK> requires (detail::transparent<Compare> && 
                                    !std::is_convertible_v<KK, iterator> && 
                                    !std::is_convertible_v<KK, const_iterator>)
    size_type erase(KK& x)
    {
        return erase_by_key(x);
    }

    using insert_functions::insert;

    auto insert(node_type&& nh)
    {
        return insert_container_node_type(std::move(nh));
    }

    auto insert(const_iterator pos, node_type&& nh)
    {
        if constexpr (UniqueKey)
        {
            return insert(std::move(nh)).position;
        }
        else
        {
            return insert(std::move(nh));
        }
    }

    bool operator==(const tree& other) const
    {
        return size() == other.size() && std::equal(begin(), end(), other.begin());
    }

    auto operator<=>(const tree& other) const
    {
        return std::lexicographical_compare_three_way(
            begin(), end(), other.begin(), other.end());
    }

    void swap(tree& other) noexcept(IsNothrowSwap)
    {
        using std::swap;

        if constexpr (alloc_traits::propagate_on_container_swap::value)
        {
            swap(m_alloc, other.m_alloc);
        }
        else
        {
            assert(m_alloc == other.m_alloc && "It's undefined behavior if the allocators are unequal here.");
        }

        // swap header
        swap_header(other);

        // swap other member fields
        swap(m_cmp, other.m_cmp);
        swap(m_size, other.m_size);
    }

    // a valid iterator into this container
    node_type extract(const_iterator position)
    {
        return extract_node(position.base().link());
    }

    node_type extract(const key_type& x)
    {
        return extract(find((x)));
    }

    template <typename K>
        requires detail::transparent<Compare>
    node_type extract(K&& x)
    {
        return extract(find((x)));
    }

    template <typename C2, bool U2>
    void merge(tree<KeyOfValue, C2, Allocator, U2, Node>& source)
    {
        // We check the address directly to avoid self-merge.
        if (static_cast<const void*>(this) == static_cast<const void*>(std::addressof(source)))
        {
            return;
        }

        assert(get_allocator() == source.get_allocator() && "The allocators should be equal.");
        merge_tree_node(source);
    }

    template <typename C2, bool U2>
    void merge(tree<KeyOfValue, C2, Allocator, U2, Node>&& source)
    {
        merge(source);
    }

    template <typename Self>
    copy_const_t<Self, node_base*> header(this Self& self)
    {
        return std::addressof(self.m_header);
    }

protected:

    template <typename C2, bool U2>
    void merge_tree_node(tree<KeyOfValue, C2, Allocator, U2, Node>& source)
    {
        if (source.empty())
        {
            return;
        }

        if constexpr (UniqueKey)
        {
            node_base* cur = source.header()->lchild();;

            while (!cur->is_header())
            {
                auto next = cur->increment();
                auto [x, p] = get_insert_unique_pos(keys(cur));

                if (p != nullptr)
                {
                    auto y = source.extract_node(cur);
                    auto z = std::exchange(y.m_ptr, nullptr);
                    insert_node(x, p, z);
                }

                cur = next;
            }
        }
        else
        {
            // For multimap/multiset, we can directly merge the tree since 
            // all nodes in source tree will be inserted into this tree.
            auto node = source.header()->parent();
            dfs_merge_node(node);
            source.make_header_sentinel();
            source.clear();
        }
    }

    void dfs_merge_node(node_base* x)
    {
        if (x)
        {
            dfs_merge_node(x->lchild());
            dfs_merge_node(x->rchild());

            auto [p, insert_left] = get_insert_pos(keys(x));
            
            reset_node(x);
            insert_node(insert_left, p, x);
        }
    }

    node_type extract_node(node_base* node)
    {
        if (node == std::addressof(m_header))
        {
            return node_type(nullptr, m_alloc);
        }
        else
        {
            // unlink and reset the node   
            node->rebalance_for_erase(m_header);
            reset_node(node);
            --m_size;
            return node_type(static_cast<tree_node*>(node), m_alloc);
        }
    }

    // Some helper functions for copy/move/swap elements from another tree
    template <typename Tree, typename Fn>
    void copy_or_move_from_another_tree(Tree&& other, Fn fn) 
    {
        // Reset current tree
        make_header_sentinel();

        if (other.empty())
        {
            // If t is empty, we do nothing and just return
            return;
        }
        else
        {
            // dfs_copy_or_move may throw exception!
            auto root = dfs_copy_or_move(
                header()->parent(), 
                header(), 
                other.header()->parent(), 
                fn);
    
            header()->parent(root);
            header()->lchild(header()->parent()->minimum());
            header()->rchild(header()->parent()->maximum());
        }

        // Keep size
        m_size = other.m_size;
    }

    void stolen_header(tree& other)
    {
        header()->parent(other.header()->parent());
        header()->lchild(other.header()->lchild());
        header()->rchild(other.header()->rchild());
        other.make_header_sentinel();
    }

    void swap_header(tree& other)
    {
        if (empty() && other.empty())
        {
            return;
        }
        else if (empty())
        {
            stolen_header(other);
        }
        else if (other.empty())
        {
            other.stolen_header(*this);
        }
        else
        {
            auto root1 = header()->parent();
            auto leftmost1 = header()->lchild();
            auto rightmost1 = header()->rchild();

            auto root2 = other.header()->parent();
            auto leftmost2 = other.header()->lchild();
            auto rightmost2 = other.header()->rchild();

            header()->parent(root2);
            header()->lchild(leftmost2);
            header()->rchild(rightmost2);

            other.header()->parent(root1);
            other.header()->lchild(leftmost1);
            other.header()->rchild(rightmost1);
        }
    }

    // Copy and move helper
    template <typename Fn>
    node_base* dfs_copy_or_move(node_base*& x, node_base* p, node_base* y, Fn fn)
    {
        if (!y)
        {
            return nullptr;
        }

        x = create_node(fn(static_cast<tree_node*>(y)->value()));
        x->clone(y);
        x->lchild(dfs_copy_or_move(x->lchild(), x, y->lchild(), fn));
        x->rchild(dfs_copy_or_move(x->rchild(), x, y->rchild(), fn));
        x->parent(p);
        return x;
    }

    // Lookup helper
    template <typename K>
    iterator lower_bound_impl(const K& k) 
    {
        auto y = &m_header, x = header()->parent();

        while (x)
        {
            if (!m_cmp(keys(x), k))
            {
                y = x, x = x->lchild();
            }
            else
            {
                x = x->rchild();
            }
        }
        return { y };
    }

    template <typename K>
    iterator upper_bound_impl(const K& k) 
    {
        auto y = &m_header, x = header()->parent();

        while (x)
        {
            if (m_cmp(k, keys(x)))
            {
                y = x, x = x->lchild();
            }
            else
            {
                x = x->rchild();
            }
        }
        return { y };
    }

    // Insert helpers
    std::conditional_t<UniqueKey, insert_return_type, iterator> insert_container_node_type(node_type&& nh)
    {
        if constexpr (UniqueKey)
        {
            if (nh.empty())
            {
                return insert_return_type(end(), false, node_type(nullptr, m_alloc));
            }
    
            tree_node* node = std::exchange(nh.m_ptr, nullptr);
            auto [x, p] = get_insert_unique_pos(keys(node));
    
            return p == nullptr ? 
                insert_return_type(iterator(x), false, node_type(node, m_alloc)) : 
                insert_return_type(insert_node(x, p, node->base()), true, node_type(nullptr, m_alloc));
        }
        else
        {
            if (nh.empty())
            {
                return end();
            }

            tree_node* node = std::exchange(nh.m_ptr, nullptr);
            auto [p, insert_left] = get_insert_pos(keys(node));
            return insert_node(insert_left, p, node->base());
        }
    }    

    template <typename Arg>
    std::pair<iterator, bool> insert_unique(Arg&& arg)
    {
        auto [x, p] = get_insert_unique_pos(KeyOfValue()(arg));
        return p 
             ? std::make_pair(insert_value(x, p, (Arg&&)arg), true)  
             : std::make_pair(x, false);
    }

    template<typename Arg>
    iterator insert_value(node_base* x, node_base* p, Arg&& arg)
    {
        bool insert_left = (x != 0 || p == &m_header
            || m_cmp(KeyOfValue()(arg), keys(p)));

        auto z = create_node((Arg&&)arg);

        z->insert_and_rebalance(insert_left, p, m_header);
        ++m_size;
        return iterator(z);
    }

    iterator insert_node(node_base* x, node_base* p, node_base* node)
    {
        bool insert_left = (x != 0 || p == &m_header || m_cmp(keys(node), keys(p)));

        node->insert_and_rebalance(insert_left, p, m_header);
        ++m_size;
        return iterator(node);
    }

    iterator insert_node(bool insert_left, node_base* p, node_base* node)
    {
        node->insert_and_rebalance(insert_left, p, m_header);
        ++m_size;
        return iterator(node);
    }

    template <typename K>
    iterator insert_multi(const K& value)
    {
        auto [p, insert_left] = get_insert_pos(value);
        return insert_node(insert_left, p, create_node(value));
    }

    template <typename K>
    std::pair<node_base*, bool> get_insert_pos(const K& k)
    {
        auto y = &m_header, x = header()->parent();
        bool comp = true;

        while (x)
        {
            y = x;
            comp = m_cmp(k, keys(x)); 
            x = comp ? x->lchild() : x->rchild();
        }

        return { y, comp };
    }

    // Find the position that the x will be inserted
    template <typename K>
    std::pair<node_base*, node_base*> get_insert_unique_pos(const K& k)
    {
        auto y = &m_header, x = header()->parent();
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

        if (m_cmp(KeyOfValue()(*j), k))
        {
            return { x, y };
        }

        return { j.link(), nullptr };
    }

    // Remove helpers
    void erase_by_node(node_base* x)
    {      
        x->rebalance_for_erase(m_header);
        drop_node(static_cast<tree_node*>(x));
        m_size--;
    }

    template <typename K>
    size_type erase_by_key(const K& x)
    {
        if constexpr (UniqueKey)
        {
            iterator iter = this->find(x);
    
            if (iter != end())
            {
                erase_by_node(iter.link());
                return 1;
            }
    
            return 0;
        }
        else
        {
            auto [first, last] = this->equal_range(x); 
            size_type cnt = 0;

            for (; first != last; first = erase(first), ++cnt);
            
            return cnt;
        }
    }

    void reset()
    {
        dfs_destroy(header()->parent());
        make_header_sentinel();
        m_size = 0;
    }

    void make_header_sentinel()
    {
        header()->parent(nullptr);
        header()->lchild(header());
        header()->rchild(header());
        header()->as_empty_tree_header();
    }

    // Create a new node
    template <typename... Args>
    tree_node* create_node(Args&&... args)
    {
        auto node = alloc_node();
        construct_node(node, (Args&&)args...);
        return node;
    }

    // Allocate memory for a new node
    tree_node* alloc_node()
    {
        return allocator_adaptor<node_allocator>::allocate(m_alloc, 1);
    }

    // Construct a new node
    template <typename... Args>
    void construct_node(tree_node* node, Args&&... args)
    {
        try
        {
            reset_node(node);
            allocator_adaptor<node_allocator>::construct(m_alloc, node->value_ptr(), (Args&&)args...);
        }
        catch (...)
        {
            // if exception occurs, deallocate memory and rethrow the exception
            dealloc_node(node);
            throw;
        }
    }

    // Destroy all nodes by postorder traversal
    void dfs_destroy(node_base* node)
    {
        if (node)
        {
            dfs_destroy(node->lchild());
            dfs_destroy(node->rchild());
            drop_node(static_cast<tree_node*>(node));
        }
    }

    // Destroy node and deallocate memory
    void drop_node(tree_node* node)
    {
        destroy_node(node);
        dealloc_node(node);
    }

    // Destroy node
    void destroy_node(tree_node* node)
    {
        allocator_adaptor<node_allocator>::destroy(m_alloc, node->value_ptr());
    }

    // Deallocate memory
    void dealloc_node(tree_node* node)
    {
        allocator_adaptor<node_allocator>::deallocate(m_alloc, node, 1);
    }

    // Reset node
    void reset_node(node_base* node)
    {
        node->parent(nullptr);
        node->lchild(nullptr);
        node->rchild(nullptr);
        node->init();
    }

protected:
    
    static const key_type& keys(const tree_node* node)
    {
        return KeyOfValue()(node->value());
    }
    
    static const key_type& keys(const node_base* node)
    {
        return keys(static_cast<const tree_node*>(node));
    }

    [[no_unique_address]] Compare m_cmp;
    [[no_unique_address]] Allocator m_alloc;
    node_base m_header;
    size_type m_size;
};

/**
 * @brief A tree-based associative container that associates keys with values.
 *
 * The set and map container share many of the same member functions, so we
 * implement them in a common base class. The derived classes will only need to
 * implement the methods which not common to both set and map.
 *  
 * @param K Key type
 * @param V Value type
 * @param Compare Key comparison function object
 * @param Allocator Type of the allocator object used to define the storage allocation model
 * @param UniqueKey Whether the container is unique-key or multi-key
 * @param Node Type of tree node with basic tree operations but value field
 */
// template <typename K, typename V, typename Compare, typename Allocator, bool UniqueKey, typename Node>
template <typename KeyOfValue, typename Compare, typename Allocator, bool UniqueKey, typename Node>
class associative_tree : public tree<KeyOfValue, Compare, Allocator, UniqueKey, Node>
{
    using base = tree<KeyOfValue, Compare, Allocator, UniqueKey, Node>;
    
    // Same name as base class, is there any better choice with using?
    template <typename U> 
    using key_arg_t = base::template key_arg_t<U>;

public:

    using base::base;
    using base::operator=;
    using typename base::value_type;
    using typename base::key_type;
    using typename base::iterator;
    using typename base::const_iterator;
    using mapped_type = typename KeyOfValue::mapped_type;

    struct value_compare : ordered_map_container_value_compare<value_type, Compare>
    {
    protected:
        friend class associative_tree; 

        value_compare(Compare compare) : ordered_map_container_value_compare<value_type, Compare>(compare) { }
    };

    value_compare value_comp() const
    {
        return value_compare(this->m_cmp);
    }

    using base::insert;

    // Extend two insert methods
    template <std::convertible_to<value_type> PairLike> 
    iterator insert(PairLike&& value)
    {
        // This overload is equivalent to emplace(std::forward<P>(value)) and only participates 
        // in overload resolution if std::is_constructible<value_type, P&&>::value == true
        return this->emplace((PairLike&&) value);
    }

    template <std::convertible_to<value_type> PairLike> 
    iterator insert(const_iterator pos, PairLike&& value)
    {
        return insert((PairLike&&) value);
    }

public:

    // map::operator[]
    mapped_type& operator[](const key_type& key) requires (UniqueKey)
    {
        return operator_square_brackets_impl(key);
    }

    mapped_type& operator[](key_type&& key) requires (UniqueKey)
    {
        return operator_square_brackets_impl(std::move(key));
    }

    template <typename KK>
        requires (detail::transparent<Compare> && UniqueKey)
    mapped_type& operator[](KK&& x)
    {
        return operator_square_brackets_impl((KK&&)x);
    }

    // map::at
    template <typename Key = key_type> 
        requires (UniqueKey)
    mapped_type& at(const key_arg_t<Key>& x) 
    {
        auto it = this->find(x);
        return it != this->end() ? 
               it->second : throw std::out_of_range("The container does not have an element with the specified key.");
    }

    template <typename Key = key_type> 
        requires (UniqueKey)
    const mapped_type& at(const key_arg_t<Key>& x) const
    {
        return const_cast<associative_tree*>(this)->at(x);
    }

    // map::try_emplace
    template <typename... Args> 
        requires (UniqueKey)
    std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args)
    {
        return try_emplace_impl(key, (Args&&)args...);
    }

    template <typename... Args> 
        requires (UniqueKey)
    std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args)
    {
        return try_emplace_impl(std::move(key), (Args&&)args...);
    }

    template <typename... Args> 
        requires (UniqueKey)
    iterator try_emplace(const_iterator hint, const key_type& key, Args&&... args)
    {
        return try_emplace(key, (Args&&)args...);
    }

    template <typename... Args> 
        requires (UniqueKey)
    iterator try_emplace(const_iterator hint, key_type&& key, Args&&... args)
    {
        return try_emplace(key, (Args&&)args...);
    }

    // If equal_range(u.first) == equal_range(k) is false, the behavior 
    // is undefined, where u is the new element to be inserted.
    template <typename KK, typename... Args>
        requires (detail::transparent<Compare> && 
                 !std::is_convertible_v<KK, iterator> && 
                 !std::is_convertible_v<KK, const_iterator> &&
                  UniqueKey)
    std::pair<iterator, bool> try_emplace(KK&& key, Args&&... args)
    {
        return try_emplace_impl((KK&&)key, (Args&&)args...);
    }

    template <typename KK, typename... Args>
        requires (detail::transparent<Compare> && 
                 !std::is_convertible_v<KK, iterator> && 
                 !std::is_convertible_v<KK, const_iterator> &&
                  UniqueKey)
    iterator try_emplace(const_iterator hint, KK&& key, Args&&... args)
    {
        return try_emplace((KK&&)key, (Args&&)args...);
    }

    // map::insert_or_assign
    template <typename... Args> 
        requires (UniqueKey)
    std::pair<iterator, bool> insert_or_assign(const key_type& key, Args&&... args)
    {
        return insert_or_assign(key, (Args&&)args...);
    }

    template <typename... Args> 
        requires (UniqueKey)
    std::pair<iterator, bool> insert_or_assign(key_type&& key, Args&&... args)
    {
        return insert_or_assign_impl(std::move(key), (Args&&)args...);
    }

    template <typename... Args> 
        requires (UniqueKey)
    iterator insert_or_assign(const_iterator hint, const key_type& key, Args&&... args)
    {
        return insert_or_assign(key, (Args&&)args...);
    }

    template <typename... Args> 
        requires (UniqueKey)
    iterator insert_or_assign(const_iterator hint, key_type&& key, Args&&... args)
    {
        return insert_or_assign(key, (Args&&)args...);
    }

    // If equal_range(u.first) == equal_range(k) is false, the behavior 
    // is undefined, where u is the new element to be inserted.
    template <typename KK, typename... Args>
        requires (detail::transparent<Compare> && 
                 !std::is_convertible_v<KK, iterator> && 
                 !std::is_convertible_v<KK, const_iterator> &&
                  UniqueKey)
    std::pair<iterator, bool> insert_or_assign(KK&& key, Args&&... args)
    {
        return insert_or_assign_impl((KK&&)key, (Args&&)args...);
    }

    template <typename KK, typename... Args>
        requires (detail::transparent<Compare> && 
                 !std::is_convertible_v<KK, iterator> && 
                 !std::is_convertible_v<KK, const_iterator> &&
                  UniqueKey)
    iterator insert_or_assign(const_iterator hint, KK&& key, Args&&... args)
    {
        return insert_or_assign((KK&&)key, (Args&&)args...);
    }

protected:

    template <typename KK>
    mapped_type& operator_square_brackets_impl(KK&& k)
    {
        auto [x, p] = this->get_insert_unique_pos(k);
        
        if (p) 
        {
            auto z = this->create_node((KK&&)k, mapped_type());
            return this->insert_node(x, p, z)->second;
        }
        
        auto j = iterator(x);
        return j->second;
    }

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
        j->second = (M&&)obj;
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

template <typename Node, typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using tree_set = tree<identity<T>, Compare, Allocator, true, Node>;

template <typename Node, typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using tree_multiset = tree<identity<T>, Compare, Allocator, false, Node>;

template <typename Node, typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using tree_map = associative_tree<select1st<K, V>, Compare, Allocator, true, Node>;

template <typename Node, typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using tree_multimap = associative_tree<select1st<K, V>, Compare, Allocator, false, Node>;

}  // namespace cpp::collections