#pragma once

#include <utility>
#include <iostream>
#include <array>
#include <compare>
#include <ranges>
#include <iterator>
#include <format>
#include <set>

#include <assert.h>


namespace detail
{

// https://en.cppreference.com/w/cpp/ranges/to
template <typename R, typename T>
concept container_compatible_range = 
    std::ranges::input_range<R> 
    && std::convertible_to<std::ranges::range_reference_t<R>, T>;

template <typename T, typename... Args>
struct emplace_helper
{
    static constexpr bool value = []() 
    {
        if constexpr (sizeof...(Args) != 1)
            return false;
        else
            return std::conjunction_v<std::is_same<T, std::remove_cvref_t<Args>>...>;
    }();
};

template <typename T>
concept has_transparent = requires { typename T::is_transparent; };

template <typename... Ts>
concept transparent = (has_transparent<Ts> && ...); 

template <bool IsTransparent>
struct key_arg_helper
{
    template <typename K1, typename K2>
    using type = K1;
};

template <>
struct key_arg_helper<false>
{
    template <typename K1, typename K2>
    using type = K2;
};

template <bool IsTransparent, typename K1, typename K2>
using key_arg = typename key_arg_helper<IsTransparent>::template type<K1, K2>;

} // namespace detail

// Is param T necessary? Maybe we can just access by typename Allocator::value_type?
template <typename T, typename Allocator>
struct value_handle
{
    static_assert(std::is_same_v<T, std::remove_cvref_t<T>>);
    static_assert(std::is_same_v<Allocator, std::remove_cvref_t<Allocator>>);

    using allocator_type = Allocator;
    using alloc_traits = std::allocator_traits<allocator_type>;
    // using pointer = T*;
    using value_type = T;

    template <typename... Args>
    value_handle(const allocator_type& alloc, Args&&... args)
        : m_alloc(alloc)
    {
        alloc_traits::construct(m_alloc, reinterpret_cast<T*>(&m_raw), (Args&&)args...);
    }

    value_handle(const value_handle&) = delete;
    value_handle(value_handle&&) = delete;

    ~value_handle()
    {
        alloc_traits::destroy(m_alloc, reinterpret_cast<T*>(&m_raw));
    }

    value_type&& operator*()
    {
        return std::move(*reinterpret_cast<T*>(&m_raw));
    }

    [[no_unique_address]] allocator_type m_alloc;
    alignas(T) unsigned char m_raw[sizeof(T)];
};

template <typename NodeAllocator>
struct node_handle_base
{
    using allocator_type = NodeAllocator;
    using ator_traits  = std::allocator_traits<allocator_type>;
    using pointer = typename ator_traits::pointer;
    using container_node_type = typename std::pointer_traits<pointer>::element_type;

    static constexpr bool IsNothrowSwap = 
            ator_traits::propagate_on_container_swap::value || 
            ator_traits::is_always_equal::value;

    constexpr node_handle_base() = default;

    explicit node_handle_base(pointer ptr, const allocator_type& alloc)
        : m_ptr(ptr), m_alloc(alloc)
    {
    }

    // Node handles are move-only, the copy assignment is not defined.
    node_handle_base(const node_handle_base&) = delete;

    node_handle_base(node_handle_base&& nh) noexcept
        : m_ptr(std::exchange(nh.m_ptr, nullptr))
    {
        m_alloc.swap(nh.m_alloc);
    }

    node_handle_base& operator=(node_handle_base&& nh) noexcept
    {
        if (this != std::addressof(nh))
        {
            // If the node handle is not empty, destroys the value_type subobject 
            // and deallocates the container element 
            if (!empty())
            {
                reset();
            }

            // Acquires ownership of the container element from nh
            m_ptr = std::exchange(nh.m_ptr, nullptr);

            // If node handle was empty or if POCMA is true, move assigns the allocator from nh
            if constexpr (ator_traits::propagate_on_container_move_assignment::value)
            {
                *m_alloc = std::move(*nh.m_alloc);
            }
            else
            {
                assert(m_alloc == nh.m_alloc && "Undefined behaviour");
            }

            // Sets nh to the empty state.
            nh.reset();
        }
        return *this;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_ptr == nullptr;
    }

    explicit operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }

    void reset()
    {
        if (!empty())
        {
            ator_traits::destroy(*m_alloc, m_ptr->value_ptr());
            ator_traits::deallocate(*m_alloc, m_ptr, 1);
            m_alloc.reset();
            m_ptr = nullptr;
        }
    }

    allocator_type get_allocator() const
    {
        return *m_alloc;
    }

    ~node_handle_base()
    {
        if (!empty())
        {
            reset();
        }
    }

    void swap(node_handle_base& nh) noexcept(IsNothrowSwap)
    {
        using std::swap;
        swap(m_ptr, nh.m_ptr);
        if constexpr (ator_traits::propagate_on_container_swap::value)
        {
            m_alloc.swap(nh);
        }
    }

    friend void swap(node_handle_base& x, node_handle_base& y) 
        noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }

    pointer m_ptr = nullptr;

    /**
     * std::optional may not a good choice since it contains a boolean filed to check 
     * the current state of the object, which is not necessary for this case. 
     */
    std::optional<allocator_type> m_alloc;
};

template <typename Iterator, typename NodeHandle>
struct node_insert_return
{
    Iterator position;
    bool inserted;
    NodeHandle node;
};

struct node_interface
{
    template <typename Node>
    constexpr auto& parent(this Node& self)
    {
        return self.m_link[0];
    }

    template <typename Node>
    constexpr auto& lchild(this Node& self)
    {
        return self.m_link[1];
    }

    template <typename Node>
    constexpr auto& rchild(this Node& self)
    {
        return self.m_link[2];
    }

    template <typename Node>
    constexpr void parent(this Node& self, std::type_identity_t<Node>* node)
    {
        self.m_link[0] = node;
    }

    template <typename Node>
    constexpr void lchild(this Node& self, std::type_identity_t<Node>* node)
    {
        self.m_link[1] = node;
    }

    template <typename Node>
    constexpr void rchild(this Node& self, std::type_identity_t<Node>* node)
    {
        self.m_link[2] = node;
    }

    template <typename Node>
    constexpr Node* maximum(this Node& self)
    {
        auto x = std::addressof(self);
        for (; x->rchild(); x = x->rchild());
        return x;
    }

    template <typename Node>
    constexpr Node* minimum(this Node& self)
    {
        auto x = std::addressof(self);
        for (; x->lchild(); x = x->lchild());
        return x;
    }

    template <typename Node>
    constexpr Node* decrement(this Node& self)
    {
        auto x = std::addressof(self);

        if (x->lchild())
        {
            return x->lchild()->maximum();
        }
        else
        {
            auto y = x->parent();

            while (x == y->lchild())
            {
                x = y;
                y = y->parent();
            }

            if (x->lchild() != y)
            {
                x = y;
            }

            return x;
        }
    }

    template <typename Node>
    constexpr Node* increment(this Node& self)
    {
        auto x = std::addressof(self);

        if (x->rchild())
        {
            return x->rchild()->minimum();
        }
        else
        {
            auto y = x->parent();

            while (x == y->rchild())
            {
                x = y;
                y = y->parent();
            }

            if (x->rchild() != y)
            {
                x = y;
            }

            return x;
        }
    }

    /*
    *     x            y              
    *       \   =>   /    
    *         y    x
    */
    template <typename Node>    
    constexpr void rotate_left(this Node& self, Node*& root)
    {
        auto x = std::addressof(self);
        auto y = x->rchild();

        x->rchild(y->lchild());

        if (y->lchild())
        {
            y->lchild()->parent(x);
        }

        y->parent(x->parent());

        // x->parent will never be nullptr, since header->parent == root 
        // and root->parent == header
        if (x == root)
        {
            root = y;
        }
        else if (x == x->parent()->lchild()) 
        {
            x->parent()->lchild(y);
        }
        else
        {
            x->parent()->rchild(y);
        }
        
        y->lchild(x);
        x->parent(y);
    }

    /*
    *     x            y
    *   /   =>       \
    *   
    *  y              x
    */
    template <typename Node>
    constexpr void rotate_right(this Node& self, Node*& root)
    {
        auto x = std::addressof(self);
        auto y = x->lchild();

        x->lchild(y->rchild());
        
        if (y->rchild())
        {
            y->rchild()->parent(x);
        }

        y->parent(x->parent());

        // x->parent will never be nullptr, since header->parent == root 
        // and root->parent == header
        if (x == root)
        {
            root = y;
        }
        else if (x == x->parent()->rchild())
        {
            x->parent()->rchild(y);
        }
        else
        {
            x->parent()->lchild(y);
        }
        
        y->rchild(x);
        x->parent(y);
    }

};

struct avl_node : public node_interface
{
    static constexpr int balance_factor = 2;

    // Nodes
    avl_node* m_link[3];

    // Height(balance factor) of current node, -1 for header, 1 for leaf and 0 for nullptr
    int m_height;

    std::string to_string() const
    {
        return std::format("({})", m_height);
    }

    // Initialize node without value field after calling allocate
    constexpr void init()
    {
        m_height = 1;
    }

    // Reset header for an empty tree 
    constexpr void as_empty_tree_header()
    {
        m_height = -1;
    }

    static constexpr int height(const avl_node* node)
    {
        return node ? node->m_height : 0;
    }

    constexpr void update_height()
    {
        int lh = height(lchild());
        int rh = height(rchild());
        m_height = std::max(lh, rh) + 1;
    }

    constexpr void clone(const avl_node* x)
    {
        m_height = x->m_height;
    }

    constexpr bool is_header() const
    {
        return m_height == -1;
    }

    constexpr void erase_node(avl_node* header)
    {
        auto x = this;

        auto& [root, leftmost, rightmost] = header->m_link;

        avl_node* child = nullptr;
        avl_node* parent = nullptr; // for rebalance

        if (x->lchild() && x->rchild())
        {
            auto successor = x->rchild()->minimum();
            child = successor->rchild();
            parent = successor->parent();

            if (child)
            {
                child->parent(parent);
            }

            successor->parent()->lchild() == successor 
                ? successor->parent()->lchild(child)
                : successor->parent()->rchild(child);

            if (successor->parent() == x)
            {
                parent = successor;
            }
            
            successor->lchild(x->lchild());
            successor->rchild(x->rchild());
            successor->parent(x->parent());
            successor->m_height = x->m_height;
        
            if (x == root)
            {
                root = successor;
            }
            else
            {
                x->parent()->lchild() == x 
                    ? x->parent()->lchild(successor)
                    : x->parent()->rchild(successor);
            }

            x->lchild()->parent(successor);

            if (x->rchild())
            {
                x->rchild()->parent(successor);
            }
        }
        else
        {
            // update leftmost or rightmost
            if (!x->lchild() && !x->rchild())
            {
                // leaf, such as just one root
                if (x == leftmost)
                {
                    leftmost = x->parent();
                }
                if (x == rightmost)
                {
                    rightmost = x->parent();
                }
            }
            else if (x->lchild())
            {
                // only left child
                child = x->lchild();
                if (x == rightmost)
                {
                    rightmost = child->maximum();
                }
            }
            else
            {
                // only right child
                child = x->rchild();
                if (x == leftmost)
                {
                    leftmost = child->minimum();
                }
            }

            if (child)
            {
                child->parent(x->parent());
            }
            if (x == root)
            {
                root = child;
            }
            else
            {
                x->parent()->lchild() == x 
                    ? x->parent()->lchild(child)
                    : x->parent()->rchild(child);
            }

            parent = x->parent();
        }
        parent->avl_tree_rebalance_erase(header);
    }

    constexpr void avl_tree_fix_l(avl_node* header)
    {
        auto x = this;
        auto r = x->rchild();
        int lh0 = height(r->lchild());
        int rh0 = height(r->rchild());

        if (lh0 > rh0)
        {
            r->rotate_right(header->parent());
            r->update_height();
            r->parent()->update_height();
        }

        x->rotate_left(header->parent());
        x->update_height();
        x->parent()->update_height();
    }

    constexpr void avl_tree_fix_r(avl_node* header)
    {
        auto x = this;
        auto l = x->lchild();
        int lh0 = height(l->lchild());
        int rh0 = height(l->rchild());

        if (lh0 < rh0)
        {
            l->rotate_left(header->parent());
            l->update_height();
            l->parent()->update_height();
        }

        x->rotate_right(header->parent());
        x->update_height();
        x->parent()->update_height();
    }

    constexpr void avl_tree_rebalance_insert(avl_node* header)
    {
        auto x = this;

        for (x = x->parent(); x != header; x = x->parent())
        {
            int lh = height(x->lchild());
            int rh = height(x->rchild());
            int h = std::max(lh, rh) + 1;

            if (height(x) == h) 
            {
                break;
            }
            
            x->m_height = h;

            int diff = lh - rh;

            if (diff <= -2)
            {
                x->avl_tree_fix_l(header);
            }
            else if (diff >= 2)
            {
                x->avl_tree_fix_r(header);
            }
        }
    }

    constexpr void avl_tree_rebalance_erase(avl_node* header)
    {
        auto x = this;

        for (; x != header; x = x->parent())
        {
            int lh = height(x->lchild());
            int rh = height(x->rchild());
            int h = std::max(lh, rh) + 1;
            x->m_height = h;
            int diff = lh - rh;
            
            if (x->m_height != h)
            {
                x->m_height = h;
            }
            else if (-1 <= diff && diff <= 1) 
            {
                break;
            }

            if (diff <= -2)
            {
                x->avl_tree_fix_l(header);
            }
            else 
            {
                x->avl_tree_fix_r(header);
            }
        }
    }

    constexpr void insert_and_rebalance(bool insert_left, avl_node* p, avl_node& header)
    {
        auto x = this;

        x->parent(p);
        x->lchild(nullptr);
        x->rchild(nullptr);
        x->m_height = 1;

        if (insert_left)
        {
            p->lchild(x);

            if (p == &header)
            {
                header.parent(x);
                header.rchild(x);
            }
            else if (p == header.lchild())
            {
                header.lchild(x);
            }
        }
        else
        {
            p->rchild(x);

            if (p == header.rchild())
            {
                header.rchild(x);
            }
        }

        // rebalance
        x->avl_tree_rebalance_insert(&header);
    }

    constexpr avl_node* rebalance_for_erase(avl_node& header)
    {
        auto z = this;
        z->erase_node(&header);
        return z;
    }
    
};

template <typename Node, typename T>
struct value_field : public Node
{
    T m_value;

    value_field(const value_field&) = delete;

    T* value_ptr()
    {
        return std::addressof(m_value);
    }

    const T* value_ptr() const
    {
        return std::addressof(m_value);
    }

    Node* base()
    {
        return static_cast<Node*>(this);
    }

    const Node* base() const
    {
        return static_cast<const Node*>(this);
    }
};

template <typename T>
struct identity
{
    using key_type = T;

    using value_type = T;

    template <typename U>
    static constexpr auto&& operator()(U&& x)
    {
        return (U&&)x;
    }
};

template <typename T1, typename T2>
struct select1st
{
    using key_type = T1;

    using value_type = std::pair<const T1, T2>;

    template <typename U>
    static constexpr auto&& operator()(U&& x)
    {
        return ((U&&)x).first;
    }
};

template <typename KeyValue,
    typename Compare,
    typename Allocator,
    bool UniqueKey, typename Node>
class tree 
{
    static_assert(UniqueKey, "Non-unique keys are not supported");

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

private:
public:

    using tree_node = value_field<Node, value_type>;

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
        constexpr tree_iterator base(this tree_iterator it)
        {
            return it;
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

    using node_type = node_handle_base<node_allocator>;
    using insert_return_type = node_insert_return<tree_iterator, node_type>;

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

    template <detail::container_compatible_range<value_type> R> 
    tree(std::from_range_t, R&& rg, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
        : tree(std::ranges::begin(rg), std::ranges::end(rg), comp, alloc) { }
        
    template <detail::container_compatible_range<value_type> R> 
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
        return (lower == end() || m_cmp(x, *lower)) 
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
        iterator upper = (lower == end() || m_cmp(x, *lower)) ? lower : std::next(lower); 
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
        // return equal_range(x).second;
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
        return contains(x);
    }

    // Modifiers
    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        if constexpr (detail::emplace_helper<value_type, Args...>::value)
        {
            return insert_unique((Args&&)args...);
        }
        else
        {
            // we use a value_handle to help us destroy 
            value_handle<value_type, allocator_type> handle(m_alloc, (Args&&)args...);
            return insert_unique(*handle);
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
        return erase(std::make_const_iterator(pos));
    }

    size_type erase(const key_type& key)
    {
        return erase_by_key(key);
    }

    const_iterator erase(const_iterator pos)
    {
        auto ret = std::next(pos);
        erase_by_node(pos.m_ptr);
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

    std::pair<iterator, bool> insert(const value_type& value)
    {
        return emplace(value);
    }

    std::pair<iterator, bool> insert(value_type&& value)
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

    insert_return_type insert(node_type&& nh)
    {
        if (nh.empty())
        {
            return { end(), false, node_type(nullptr, m_alloc) };
        }

        tree_node* node = std::exchange(nh.m_ptr, nullptr);
        auto [x, p] = get_insert_unique_pos(*node->value_ptr());

        return p == nullptr ? 
            insert_return_type(iterator(x), false, node_type(node, m_alloc)) : 
            insert_return_type(insert_node(x, p, node->base()), true, node_type(nullptr, m_alloc));
    }

    iterator insert(const_iterator pos, node_type&& nh)
    {
        return insert(std::move(nh)).position;
    }

    template <typename K>
        requires detail::transparent<Compare>
    std::pair<iterator, bool> insert(K&& x)
    {
        return emplace((K&&)x);
    }

    template <typename K>
        requires (detail::transparent<Compare> && 
                 !std::is_convertible_v<K, iterator> && 
                 !std::is_convertible_v<K, const_iterator>)
    iterator insert(const_iterator hint, K&& x)
    {
        return emplace_hint(hint, (K&&)x);
    }

    template<detail::container_compatible_range<value_type> R>
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
        return extract_node(position.base().m_ptr);
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

private:

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

        x = create_node(fn(*static_cast<tree_node*>(y)->value_ptr()));
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

    iterator insert_node(node_base* x, node_base* p, node_base* z)
    {
        bool insert_left = (x != 0 || p == &m_header || m_cmp(keys(z), keys(p)));

        z->insert_and_rebalance(insert_left, p, m_header);
        ++m_size;
        return iterator(z);
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

        return { j.m_ptr, nullptr };
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
        iterator iter = find(x);

        if (iter != end())
        {
            erase_by_node(iter.m_ptr);
            return 1;
        }

        return 0;
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
        return KeyValue()(*node->value_ptr());
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


template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
class avl_set : public tree<identity<T>, Compare, Allocator, true, avl_node>
{
    using base_type = tree<identity<T>, Compare, Allocator, true, avl_node>;
public:
    
    using base_type::base_type;
    using base_type::operator=;
};

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
class avl_map : public tree<select1st<K, V>, Compare, Allocator, true, avl_node>
{
    using base_type = tree<select1st<K, V>, Compare, Allocator, true, avl_node>;

public:

    using base_type::base_type;
    using base_type::operator=;
};

template <typename K, typename V, typename Compare, typename Allocator, bool Unique, typename Node>
class tree_map : public tree<select1st<K, V>, Compare, Allocator, Unique, Node>
{
    using base = tree<select1st<K, V>, Compare, Allocator, Unique, Node>;

public:

    using mapped_type = V;
    using typename base::value_type;
    using typename base::iterator;
    using typename base::const_iterator;

    struct value_compare 
    {
        bool operator()(const value_type& lhs, const value_type& rhs) const
        {
            return m_comp(lhs.first, rhs.first);
        }

    protected:
        friend class tree_map;
        
        value_compare(Compare compare) : m_comp(compare) { }

        Compare m_comp;
    };

    value_compare value_comp() const
    {
        return value_compare(this->m_cmp);
    }

    V& operator[](const K &key)
    {
        return this->try_emplace(key).first->second;
    }

    V& operator[](K &&key)
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


