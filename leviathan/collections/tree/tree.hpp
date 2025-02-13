#pragma once

#include "../common.hpp"
#include "../associative_container_interface.hpp"
#include "tree_drawer.hpp"
#include "../node_handle.hpp"

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
 * @brief Template for extended forms of binary search tree.
 * 
 * This template only provide some searching methods. The actual insertion, deletion
 * and balancing operations are done by Node.
 * 
 * We use Node as our header type which contains at least three points link
 * parent, left child and right child separately, to enable constant time begin(),
 * and to the leftmost node of the tree, to enable linear time performance when
 * used with generic set algorithms (set_union, etc...). For empty tree, the left 
 * child and right child of header link to itself and parent is null.
 * 
 * When a node being deleted has two children its successor node is relinked into
 * its place, rather than copied or moved so that the only iterators invalidated
 * are those referring to the deleted node.
 * 
 * For root node(if exist), its parent always link null.
 * 
 * @param KeyValue Extractor extract key from value. identity<T> for set and select1st<K, V> for map.
 * @param Compare
 * @param Allocator
 * @param UniqueKey True for set/map and False for multiset/multimap.
 * @param Node Type of tree node with basic tree operations but value field.
*/
template <typename KeyValue, typename Compare, typename Allocator, bool UniqueKey, typename Node>
class tree : public row_drawer
{
    // static_assert(UniqueKey, "Non-unique keys are not supported");

    template <typename A, typename B, typename C, bool D, typename E>
    friend class tree;

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
    using tree_node = value_field<Node, value_type>;

protected:

    struct tree_iterator 
    {
        using link_type = Node*;
        using value_type = value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using reference = std::conditional_t<std::is_same_v<key_type, value_type>, const value_type&, value_type&>;

        link_type m_ptr;

        constexpr tree_iterator() = default;

        constexpr tree_iterator(const tree_iterator&) = default;

        constexpr tree_iterator(link_type ptr) : m_ptr(ptr) { }

        // The const_iterator and iterator may model same type, so we offer 
        // a base method to avoid if-constexpr.
        constexpr tree_iterator& base()
        {
            return *this;
        }

        constexpr const tree_iterator& base() const
        {
            return *this;
        }

        constexpr link_type link(this tree_iterator it)
        {
            return it.m_ptr;
        }

        constexpr tree_iterator up(this tree_iterator it)
        {
            return tree_iterator(it.m_ptr->parent());
        }

        constexpr tree_iterator left(this tree_iterator it) 
        {    
            return tree_iterator(it.m_ptr->lchild());
        }

        constexpr tree_iterator right(this tree_iterator it)
        {
            return tree_iterator(it.m_ptr->rchild());
        }

        constexpr tree_iterator& operator++()
        {
            // if node is header/end/sentinel, we simply make it cycle
            m_ptr = m_ptr->is_header() ? m_ptr->lchild() : m_ptr->increment();
            return *this;
        }

        constexpr tree_iterator& operator--()
        {
            // if node is header/end/sentinel, we simply make it cycle
            m_ptr = m_ptr->is_header() ? m_ptr->rchild() : m_ptr->decrement();
            return *this;
        }

        constexpr tree_iterator operator++(int)
        {
            tree_iterator tmp = *this;
            ++*this;
            return tmp;
        }

        constexpr tree_iterator operator--(int)
        {
            tree_iterator tmp = *this;
            --*this;
            return tmp;
        }

        constexpr auto operator->(this tree_iterator it)
        {
            return std::addressof(*it);
        }

        constexpr reference operator*(this tree_iterator it) 
        {
            return *(static_cast<tree_node*>(it.m_ptr)->value_ptr());
        }

        friend constexpr bool operator==(tree_iterator lhs, tree_iterator rhs) 
        {
            return lhs.m_ptr == rhs.m_ptr;
        }
    };

    using alloc_traits = std::allocator_traits<Allocator>;
    using node_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<tree_node>;
    using node_alloc_traits = std::allocator_traits<node_allocator>;
    using node_base = Node;

public:

    using node_type = node_handle<KeyValue, node_allocator>;
    using insert_return_type = node_insert_return<tree_iterator, node_type>;

protected:

    template <typename U> 
    using key_arg_t = detail::key_arg<detail::transparent<Compare>, U, key_type>;

    static constexpr bool IsNothrowMoveConstruct =
        std::is_nothrow_move_constructible_v<Compare>
        && typename alloc_traits::is_always_equal();

    static constexpr bool IsNothrowMoveAssign =
        std::is_nothrow_move_assignable_v<Compare>
        && typename alloc_traits::is_always_equal();

    static constexpr bool IsNothrowSwap =
        std::is_nothrow_swappable_v<Compare>
        && typename alloc_traits::is_always_equal();

public:

    using iterator = tree_iterator;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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
    iterator begin()
    {
        return iterator(header()->lchild());
    }

    const_iterator begin() const
    {
        return std::make_const_iterator(const_cast<tree&>(*this).begin());
    }

    iterator end()
    {
        return iterator(header());
    }

    const_iterator end() const
    {
        return std::make_const_iterator(const_cast<tree&>(*this).end());
    }

    reverse_iterator rbegin()
    {
        return std::make_reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const
    {
        return std::make_reverse_iterator(end());
    }

    reverse_iterator rend()
    {
        return std::make_reverse_iterator(begin());
    }

    const_reverse_iterator rend() const
    {
        return std::make_reverse_iterator(begin());
    }

    const_iterator cbegin() const
    {
        return begin();
    }

    const_iterator cend() const
    {
        return end();
    }

    const_reverse_iterator crbegin() const
    {
        return rbegin();
    }

    const_reverse_iterator crend() const
    {
        return rend();
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
    template <typename K = key_type>
    iterator lower_bound(const key_arg_t<K>& x)
    {
        return lower_bound_impl(x);
    }

    template <typename K = key_type>
    const_iterator lower_bound(const key_arg_t<K>& x) const
    {
        return const_cast<tree&>(*this).lower_bound(x);
    }

    template <typename K = key_type>
    iterator find(const key_arg_t<K>& x)
    {
        iterator lower = lower_bound(x);
        return (lower == end() || m_cmp(x, keys(lower.link()))) 
              ? end() : lower;
    }

    template <typename K = key_type>
    const_iterator find(const key_arg_t<K>& x) const
    {
        return const_cast<tree&>(*this).find(x);
    }

    template <typename K = key_type>
    std::pair<iterator, iterator> equal_range(const key_arg_t<K>& x)
    {
        iterator lower = lower_bound(x);
        iterator upper;

        if constexpr (UniqueKey)
        {
            upper = (lower == end() || m_cmp(x, *lower)) ? lower : std::next(lower); 
        }
        else
        {
            upper = upper_bound(x);
        }

        return std::make_pair(lower, upper);
    }

    template <typename K = key_type>
    std::pair<const_iterator, const_iterator> equal_range(const key_arg_t<K>& x) const
    {
        return const_cast<tree&>(*this).equal_range(x);
    }

    template <typename K = key_type>
    iterator upper_bound(const key_arg_t<K>& x)
    {
        return upper_bound_impl(x);
    }

    template <typename K = key_type>
    const_iterator upper_bound(const key_arg_t<K>& x) const
    {
        return const_cast<tree&>(*this).upper_bound(x);
    }

    template <typename K = key_type>
    bool contains(const key_arg_t<K>& x) const
    {
        return find(x) != end();
    }

    template <typename K = key_type>
    size_type count(const key_arg_t<K>& x) const
    {
        if constexpr (UniqueKey)
        {
            return contains(x);
        }
        else
        {
            auto [lower, bound] = equal_range(x);
            return std::distance(lower, bound);
        }
    }

    // Modifiers
    template <typename... Args>
    std::conditional_t<UniqueKey, std::pair<iterator, bool>, iterator> emplace(Args&&... args)
    {
        if constexpr (UniqueKey)
        {
            if constexpr (detail::emplace_helper<value_type, Args...>::value)
            {
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

    template <typename... Args>
    iterator emplace_hint([[maybe_unused]] const_iterator hint, Args&&... args)
    {
        return emplace((Args&&)args...).first;
    }

    iterator erase(iterator pos) 
        requires(!std::same_as<iterator, const_iterator>)
    {
        return erase(std::make_const_iterator(pos)).base();
    }

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

    iterator erase(const_iterator first, const_iterator last)
    {
        if (first == cbegin() && last == cend()) 
        {
            clear();
        }
        else
        {
            for (; first != last; first = erase(first));
        }
        
        return last.base();
    }

    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2077r3.html
    template <typename KK> requires (detail::transparent<Compare> && 
                                    !std::is_convertible_v<KK, iterator> && 
                                    !std::is_convertible_v<KK, const_iterator>)
    size_type erase(KK& x)
    {
        return erase_by_key(x);
    }

    auto insert(const value_type& value)
    {
        return emplace(value);
    }

    auto insert(value_type&& value)
    {
        return emplace(std::move(value));
    }

    iterator insert(const_iterator pos, const value_type& value)
    {
        return emplace_hint(pos, value);
    }

    iterator insert(const_iterator pos, value_type&& value)
    {
        return emplace_hint(pos, std::move(value));
    }

    template <typename InputIt>
    void insert(InputIt first, InputIt last)
    {
        for (; first != last; ++first)
        {
            insert(*first);
        }
    }

    void insert(std::initializer_list<value_type> ilist)
    {
        insert(ilist.begin(), ilist.end());
    }

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

    template <typename K>
        requires (detail::transparent<Compare> && UniqueKey)
    std::pair<iterator, bool> insert(K&& x)
    {
        return emplace((K&&)x);
    }

    template <typename K>
        requires (detail::transparent<Compare> && 
                 !std::is_convertible_v<K, iterator> && 
                 !std::is_convertible_v<K, const_iterator> &&
                 UniqueKey)
    iterator insert(const_iterator hint, K&& x)
    {
        return emplace_hint(hint, (K&&)x);
    }

    template<container_compatible_range<value_type> R>
    void insert_range(R&& rg)
    {
        insert(std::ranges::begin(rg), std::ranges::end(rg)); 
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
    void merge(tree<KeyValue, C2, Allocator, U2, Node>& source)
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
    void merge(tree<KeyValue, C2, Allocator, U2, Node>&& source)
    {
        merge(source);
    }

protected:

    template <typename C2, bool U2>
    void merge_tree_node(tree<KeyValue, C2, Allocator, U2, Node>& source)
    {
        if (source.empty())
        {
            return;
        }

        auto node = source.header()->parent();

        if constexpr (UniqueKey)
        {
            node_base* cur = node;

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
            
            x->init();
            x->lchild(nullptr);
            x->rchild(nullptr);
            x->parent(nullptr);
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
            node->parent(nullptr);
            node->lchild(nullptr);
            node->rchild(nullptr);
            node->init();
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
        auto [x, p] = get_insert_unique_pos(KeyValue()(arg));
        return p 
             ? std::make_pair(insert_value(x, p, (Arg&&)arg), true)  
             : std::make_pair(x, false);
    }

    template<typename Arg>
    iterator insert_value(node_base* x, node_base* p, Arg&& arg)
    {
        bool insert_left = (x != 0 || p == &m_header
            || m_cmp(KeyValue()(arg), keys(p)));

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

        if (m_cmp(KeyValue()(*j), k))
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
            iterator iter = find(x);
    
            if (iter != end())
            {
                erase_by_node(iter.link());
                return 1;
            }
    
            return 0;
        }
        else
        {
            auto [first, last] = equal_range(x);
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
        node_allocator alloc(m_alloc);
        return node_alloc_traits::allocate(alloc, 1);
    }

    // Construct a new node
    template <typename... Args>
    void construct_node(tree_node* node, Args&&... args)
    {
        node_allocator alloc(m_alloc);

        try
        {
            node->parent(nullptr);
            node->lchild(nullptr);
            node->rchild(nullptr);
            node->init();
            node_alloc_traits::construct(alloc, node->value_ptr(), (Args&&)args...);
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
        node_allocator alloc(m_alloc);
        node_alloc_traits::destroy(alloc, node->value_ptr());
    }

    // Deallocate memory
    void dealloc_node(tree_node* node)
    {
        node_allocator alloc(m_alloc);
        node_alloc_traits::deallocate(alloc, node, 1);
    }

public:

    node_base* header()
    {
        return &m_header;
    }

    const node_base* header() const
    {
        return &m_header;
    }

    static const key_type& keys(const tree_node* node)
    {
        return KeyValue()(node->value());
    }

    static const key_type& keys(const node_base* node)
    {
        return keys(static_cast<const tree_node*>(node));
    }

    [[no_unique_address]] Compare m_cmp;
    [[no_unique_address]] Allocator m_alloc;
    Node m_header;
    size_type m_size;
};

template <typename K, typename V, typename Compare, typename Allocator, bool UniqueKey, typename Node>
class associative_tree : public tree<select1st<K, V>, Compare, Allocator, UniqueKey, Node>
{
    using base = tree<select1st<K, V>, Compare, Allocator, UniqueKey, Node>;

public:

    using base::base;
    using base::operator=;
    using typename base::value_type;
    using typename base::iterator;
    using typename base::const_iterator;

    using mapped_type = V;

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
};

template <typename K, typename V, typename Compare, typename Allocator, typename Node>
class associative_tree<K, V, Compare, Allocator, true, Node> : public associative_tree<K, V, Compare, Allocator, false, Node>
{
    using base = associative_tree<K, V, Compare, Allocator, false, Node>;

    // Same name as base class, is there any better choice with using?
    template <typename U> 
    using key_arg_t = base::template key_arg_t<U>;

public:

    using base::base;
    using base::operator=;
    using typename base::value_type;
    using typename base::key_type;
    using typename base::mapped_type;
    using typename base::iterator;
    using typename base::const_iterator;

    // map::operator[]
    mapped_type& operator[](const key_type& key)
    {
        return operator_square_brackets_impl(key);
    }

    mapped_type& operator[](key_type&& key)
    {
        return operator_square_brackets_impl(std::move(key));
    }

    template <typename KK>
        requires (detail::transparent<Compare>)
    mapped_type& operator[](KK&& x)
    {
        return operator_square_brackets_impl((KK&&)x);
    }

    // map::at
    template <typename Key = key_type>
    mapped_type& at(const key_arg_t<Key>& x)
    {
        auto it = this->find(x);
        return it != this->end() ? 
               it->second : throw std::out_of_range("The container does not have an element with the specified key.");
    }

    template <typename Key = key_type>
    const mapped_type& at(const key_arg_t<Key>& x) const
    {
        return const_cast<associative_tree*>(this)->at(x);
    }

    // map::try_emplace
    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const K& key, Args&&... args)
    {
        return try_emplace_impl(key, (Args&&)args...);
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(K&& key, Args&&... args)
    {
        return try_emplace_impl(std::move(key), (Args&&)args...);
    }

    template <typename... Args>
    iterator try_emplace(const_iterator hint, const K& key, Args&&... args)
    {
        return try_emplace(key, (Args&&)args...);
    }

    template <typename... Args>
    iterator try_emplace(const_iterator hint, K&& key, Args&&... args)
    {
        return try_emplace(key, (Args&&)args...);
    }

    // If equal_range(u.first) == equal_range(k) is false, the behavior 
    // is undefined, where u is the new element to be inserted.
    template <typename KK, typename... Args>
        requires (detail::transparent<Compare> && 
                 !std::is_convertible_v<KK, iterator> && 
                 !std::is_convertible_v<KK, const_iterator>)
    std::pair<iterator, bool> try_emplace(KK&& key, Args&&... args)
    {
        return try_emplace_impl((KK&&)key, (Args&&)args...);
    }

    template <typename KK, typename... Args>
        requires (detail::transparent<Compare> && 
                 !std::is_convertible_v<KK, iterator> && 
                 !std::is_convertible_v<KK, const_iterator>)
    iterator try_emplace(const_iterator hint, KK&& key, Args&&... args)
    {
        return try_emplace((KK&&)key, (Args&&)args...);
    }

    // map::insert_or_assign
    template <typename... Args>
    std::pair<iterator, bool> insert_or_assign(const K& key, Args&&... args)
    {
        return insert_or_assign(key, (Args&&)args...);
    }

    template <typename... Args>
    std::pair<iterator, bool> insert_or_assign(K&& key, Args&&... args)
    {
        return insert_or_assign_impl(std::move(key), (Args&&)args...);
    }

    template <typename... Args>
    iterator insert_or_assign(const_iterator hint, const K& key, Args&&... args)
    {
        return insert_or_assign(key, (Args&&)args...);
    }

    template <typename... Args>
    iterator insert_or_assign(const_iterator hint, K&& key, Args&&... args)
    {
        return insert_or_assign(key, (Args&&)args...);
    }

    // If equal_range(u.first) == equal_range(k) is false, the behavior 
    // is undefined, where u is the new element to be inserted.
    template <typename KK, typename... Args>
        requires (detail::transparent<Compare> && 
                 !std::is_convertible_v<KK, iterator> && 
                 !std::is_convertible_v<KK, const_iterator>)
    std::pair<iterator, bool> insert_or_assign(KK&& key, Args&&... args)
    {
        return insert_or_assign_impl((KK&&)key, (Args&&)args...);
    }

    template <typename KK, typename... Args>
        requires (detail::transparent<Compare> && 
                 !std::is_convertible_v<KK, iterator> && 
                 !std::is_convertible_v<KK, const_iterator>)
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
using tree_map = associative_tree<K, V, Compare, Allocator, true, Node>;

template <typename Node, typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using tree_multimap = associative_tree<K, V, Compare, Allocator, false, Node>;

}  // namespace leviathan::collections