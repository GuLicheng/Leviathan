// https://leetcode-cn.com/problems/design-skiplist/

#ifndef __SKIPLIST_HPP__
#define __SKIPLIST_HPP__

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <random>
#include <iterator>
#include <type_traits>


namespace leviathan 
{

    template <typename T>
    struct indentity
    {
        using iter_value_type = T;

        template <typename Compare, typename Lhs, typename Rhs>
        constexpr static bool compare(const Compare& cmp, const Lhs& lhs, const Rhs& rhs)
        { return cmp(lhs, rhs); }      
    };

    template <typename T>
    struct select1st
    {
    private:
        using _1st = std::tuple_element_t<0, T>;
        using _2nd = std::tuple_element_t<1, T>;
    public:
        using iter_value_type = std::pair<std::add_const_t<_1st>, _2nd>;

        template <typename Compare, typename Lhs, typename Rhs>
        constexpr static bool compare(const Compare& cmp, const Lhs& lhs, const Rhs& rhs)
        { return cmp(lhs.first, rhs.first); }  
    };

    template <typename Key, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key>, typename KeyTraits = indentity<Key>>
    class skip_list
    {
    public:

        constexpr static int MAXLEVEL = 32;

        template <typename Derived>
        struct header
        {
            Derived* m_prev;
            std::vector<Derived*> m_next;
            header(Derived* prev = nullptr, std::size_t num = MAXLEVEL) noexcept 
                : m_prev{ prev }, m_next{ num, nullptr } { } // make it noexcept
            
            auto derived_ptr() noexcept 
            { return static_cast<Derived*>(this); }

            auto derived_ptr() const noexcept 
            { return static_cast<const Derived*>(this); }

        };

        struct skip_node : public header<skip_node>
        {
            typename KeyTraits::iter_value_type m_data;
            using base = header<skip_node>;
            // skip_node() = default;
            skip_node(const Key &x, std::size_t nextNum)
                : base(this, nextNum), m_data(x) { }
            skip_node(Key &&x, std::size_t nextNum)
                : base(this, nextNum), m_data(std::move(x)) { }
        };
        using key_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<skip_node>;
    public:
        template <bool Const>
        struct skip_list_iterator;

        using const_iterator = skip_list_iterator<true>;
        using iterator = skip_list_iterator<false>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = reverse_iterator;
        
        using allocator_type = Allocator;
        using key_type = Key;
        using value_type = typename KeyTraits::iter_value_type; // add const for map
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using key_compare = Compare;
        using value_compare = Compare;
    
    public:
        skip_list() noexcept(noexcept(std::is_nothrow_default_constructible_v<key_allocator_type> 
            && std::is_nothrow_default_constructible_v<Compare>)) 
            : m_cmp{ }, m_alloc{ }, m_size{ 0 }, m_header{ }
        {
        }

        skip_list(Compare compare) 
            : m_cmp{ std::move(compare) }, m_alloc{ }, m_size{ 0 }, m_header{ }
        {
        }
        
        ~skip_list() noexcept 
        { clear(); }

        void clear() noexcept
        { 
            // TODO: ... for (iter = begin(), iter != end;) iter = erase(*iter); 
            auto cur = this->m_header.m_next[0];
            while (cur) 
            {
                auto next = cur->m_next[0];
                destory_one_node(cur);
                cur = next;
            }
            this->m_size = 0;
            this->m_header.m_prev = nullptr;
            std::fill_n(this->m_header.m_next.begin(), MAXLEVEL, nullptr);
        }
        void show() const
        {
            auto cur = this->m_header.m_next[0];
            while (cur) 
            {
                std::cout << cur->m_data << "("<< cur->m_next.size() << ")  ";
                cur = cur->m_next[0];
            }  
        }

        iterator begin() noexcept
        { return iterator(this->m_header.m_next[0], this); }

        iterator end() noexcept
        { return iterator(nullptr, this); }

        const_iterator begin() const noexcept
        { return const_iterator(this->m_header.m_next[0], this); }

        const_iterator end() const noexcept
        { return const_iterator(nullptr, this); }

        reverse_iterator rbegin() noexcept 
        { return std::make_reverse_iterator(end()); }

        reverse_iterator rend() noexcept
        { return std::make_reverse_iterator(begin()); }

        const_reverse_iterator rbegin() const noexcept 
        { return std::make_reverse_iterator(end()); }

        const_reverse_iterator rend() const noexcept
        { return std::make_reverse_iterator(begin()); }

        template <typename... Args>
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            auto [node, exist] = emplace_unique(this->m_header.derived_ptr(), std::forward<Args>(args)...); 
            return { iterator(node, this), exist };
        }
 
        std::pair<iterator, bool> insert(const Key& val) 
        { 
            auto [node, exist] = insert_unique(this->m_header.derived_ptr(), val); 
            return { iterator(node, this), exist };
        }

        std::pair<iterator, bool> insert(Key&& val)
        { 
            auto [node, exist] = insert_unique(this->m_header.derived_ptr(), std::move(val)); 
            return { iterator(node, this), exist };
        }

        const_iterator lower_bound(const Key& val) const
        { return const_iterator{ lower_bound_impl(this->m_header.derived_ptr(), val), this }; }

        iterator lower_bound(const Key& val) 
        { return iterator{ lower_bound_impl(this->m_header.derived_ptr(), val), this }; }

        const_iterator find(const Key& val) const 
        {
            auto [node, exist] = find_node(this->m_header.derived_ptr(), val);
            return const_iterator{ (exist ? node : nullptr), this }; 
        }

        iterator find(const Key& val) 
        {
            auto [node, exist] = find_node(this->m_header.derived_ptr(), val);
            return iterator{ (exist ? node : nullptr), this }; 
        }

        iterator erase(const Key& val)
        { return iterator{ erase_node(this->m_header.derived_ptr(), val), this }; }

        std::size_t size() const noexcept 
        { return this->m_size; }

        bool empty() const noexcept
        { return this->m_size == 0; }

    private:
        [[no_unique_address]] Compare m_cmp;
        [[no_unique_address]] key_allocator_type m_alloc;
        std::size_t m_size;
        header<skip_node> m_header;
        
        static int get_level()  
        {
            int level = 1;
            constexpr double p = 0.25;
            constexpr double rand_max = std::random_device::max();
            static std::random_device rd;
            for (; ((rd() / rand_max) < p && level < MAXLEVEL); ++level);
            return std::min(MAXLEVEL, level);

        }


        // return target node if succeed else prev position of target node for inserting
        template <bool CompletelyPrev>
        std::pair<skip_node*, bool>
        find_node_with_prev(skip_node* pos, const Key& val, std::vector<skip_node*>& prev) const noexcept
        {
            skip_node* cur{}; // cur will always be initialized
            bool exist = false;
            for (std::size_t i = pos->m_next.size() - 1; i != static_cast<std::size_t>(-1); --i)
            {
                cur = pos;
                for (; cur->m_next[i] && KeyTraits::compare(this->m_cmp, cur->m_next[i]->m_data, val); cur = cur->m_next[i]);
                auto next_node = cur->m_next[i];
                if (next_node && KeyTraits::compare(std::equal_to<>(), next_node->m_data, val)) 
                {
                    assert(KeyTraits::compare(std::equal_to<>(), next_node->m_data, val));   // UB
                    if constexpr (!CompletelyPrev)
                        return { next_node, true }; // find it and do nothing
                    else
                        exist = true;
                }
                prev[i] = cur;
            } 
            // cur is prev of node
            return { cur, exist };    
        }

        void insert_after(skip_node* pos, skip_node* new_node, const std::vector<skip_node*>& prev) noexcept
        {
            // update prev
            new_node->m_prev = pos;
            if (pos->m_next[0])
                pos->m_next[0]->m_prev = new_node;
            // update next
            for (std::size_t i = new_node->m_next.size() - 1; i != static_cast<std::size_t>(-1); --i)
            {
                new_node->m_next[i] = prev[i]->m_next[i];
                prev[i]->m_next[i] = new_node;
            }
        }

        void update_header(skip_node* pos) noexcept
        {
            // check head
            if (!pos->m_prev)
                this->m_header.m_next[0] = pos;
            // check tail
            if (!pos->m_next[0])
                this->m_header.m_prev = pos;
        }

        // find node without prev
        std::pair<skip_node*, bool>
        find_node(skip_node* p, const Key& val) const 
        {
            skip_node* cur;
            for (std::size_t i = p->m_next.size() - 1; i != static_cast<std::size_t>(-1); --i)
            {
                cur = p;
                for (; cur->m_next[i] && m_cmp(cur->m_next[i]->mapped(), KeyTraits::get(val)); cur = cur->m_next[i]);
                if (cur->m_next[i] && cur->m_next[i]->mapped() == KeyTraits::get(val))
                {
                    return { cur->m_next[i], true };
                }
            }
            return { cur, false };
        }

        template <typename U>
        std::pair<skip_node*, bool> insert_unique(skip_node* pos, U&& val)
        {
            std::vector<skip_node*> prev(MAXLEVEL, nullptr);
            auto [cur, exist] = find_node_with_prev<false>(pos, val, prev);
            if (exist) 
                return { cur, false };
            
            // insert a new node
            auto new_node = create_one_node(std::forward<U>(val));
            insert_after(cur, new_node, prev);
            update_header(new_node);
            ++this->m_size;
            return { new_node, true };
        }

        template <typename... Args>
        std::pair<skip_node*, bool> emplace_unique(skip_node* pos, Args&&... args)
        {
            auto new_node = create_one_node(std::forward<Args>(args)...);
            std::vector<skip_node*> prev(MAXLEVEL, nullptr);
            auto [cur, exist] = find_node_with_prev<false>(pos, new_node->m_data, prev);
            if (exist)
            {
                destory_one_node(new_node);
                return { cur, false };
            }
            insert_after(cur, new_node, prev);
            update_header(new_node);
            ++this->m_size;
            return { new_node, true };
        }

        skip_node* erase_node(skip_node* pos, const Key& val) 
        {
            std::vector<skip_node*> prev(MAXLEVEL, nullptr);
            auto [cur, exist] = find_node_with_prev<true>(pos, val, prev);
            if (!exist) 
                return cur->m_next[0];
            
            skip_node* deleted_node = cur->m_next[0];
            if (deleted_node->m_next[0])
                deleted_node->m_next[0]->m_prev = cur;
            for (std::size_t i = 0; i < deleted_node->m_next.size(); ++i) 
                prev[i]->m_next[i] = deleted_node->m_next[i];
            
            destory_one_node(deleted_node);
            --this->m_size;
            return prev[0]->m_next[0];
        }

        skip_node* lower_bound_impl(skip_node* pos, const Key& val) const
        {
            auto [cur, exist] = find_node(pos, val);
            return exist ? cur : cur->m_next[0];
        }

        void destory_one_node(skip_node* p) noexcept // assume destructor and deallocate is exception-safe
        {
            std::destroy_at(p);
            this->m_alloc.deallocate(p, sizeof(skip_node));
        }

        template <typename... Args>
        skip_node* create_one_node(Args&&... args)
        {
            auto level = get_level();
            auto addr = this->m_alloc.allocate(sizeof(skip_node));
            std::construct_at(addr, std::forward<Args>(args)..., level);
            return addr;
        }

    };

    template <typename Key, typename Compare, typename Allocator, typename KeyTraits>
    template <bool Const>
    struct skip_list<Key, Compare, Allocator, KeyTraits>::skip_list_iterator
    {
        using link_container_type = std::conditional_t<
            Const,
            const skip_list<Key, Compare, Allocator, KeyTraits>*,
            skip_list<Key, Compare, Allocator, KeyTraits>*>;

        using value_type = typename KeyTraits::iter_value_type;
        using link_type = std::conditional_t<Const, const skip_node*, skip_node*>;

        // using reference = const Key&;
        using reference = value_type&;
        using iterator_category	= std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        link_type m_ptr;
        link_container_type m_slist;

        constexpr skip_list_iterator() = default;
        constexpr skip_list_iterator(link_type ptr, link_container_type c) 
            : m_ptr{ ptr }, m_slist{ c } { }

        constexpr skip_list_iterator(const skip_list_iterator&) noexcept = default;

        template <bool IsConst, typename = std::enable_if_t<((Const == IsConst) || Const)>>
        constexpr skip_list_iterator(const skip_list_iterator<IsConst>& rhs) noexcept 
            : m_ptr{ rhs.m_ptr }, m_slist{ rhs.m_slist } { } 

        template <bool IsConst, typename = std::enable_if_t<((Const == IsConst) || Const)>>
        constexpr skip_list_iterator&
        operator=(const skip_list_iterator<IsConst>& rhs) noexcept 
        {
            this->m_ptr = rhs.m_ptr;
            this->m_slist = rhs.m_slist;
        }

        constexpr skip_list_iterator& operator++() 
        {
            this->m_ptr = 
                (this->m_ptr ? this->m_ptr->m_next[0] : this->m_slist->m_header.m_next[0]);
            return *this;
        }

        // std::enable_if_t<Const, skip_list_iterator&>
        constexpr skip_list_iterator& operator--() 
        {
            this->m_ptr = 
                (this->m_ptr ? this->m_ptr->m_prev : this->m_slist->m_header.m_prev);
            return *this;
        }

        constexpr skip_list_iterator operator++(int) 
        { 
            auto old = *this;
            ++ *this;
            return old;
        }

        constexpr skip_list_iterator operator--(int) 
        { 
            auto old = *this;
            -- *this;
            return old;
        }

        constexpr auto operator->() const noexcept
        { return &(this->operator*()); }

        constexpr reference operator*() const noexcept
        { return this->m_ptr->m_data; }
        
        constexpr bool operator==(const skip_list_iterator& rhs) const noexcept
        { return this->m_ptr == rhs.m_ptr; }

        constexpr bool operator!=(const skip_list_iterator& rhs) const noexcept
        { return !this->operator==(rhs); }

    };  

}


#endif