#pragma once

#include <type_traits>
#include <utility>

namespace leviathan::collections
{
    struct identity
    {
        template <typename T>
        using type = T;

        constexpr const auto& operator()(const auto& x)
        { return x; }
    };

    struct select1st
    {
        // for std::pair<const K, V>
        template <typename T>
        using type = std::remove_const_t<std::tuple_element_t<0, T>>;

        constexpr const auto& operator()(const auto& x)
        { return std::get<0>(x); }
    };

    namespace detail
    {

        // C++17 version
        // template <typename T, typename = void>
        // struct is_transparent : std::false_type { };
    
        // template <typename T>
        // struct is_transparent<T, std::void_t<typename T::is_transparent>> : std::true_type { };

        // C++20 simply use concept and require statement
        template <typename T> concept is_transparent = requires 
        { typename T::is_transparent; };

        // Non-deduced contexts
        // https://en.cppreference.com/w/cpp/language/template_argument_deduction
        // if IsTransparent is true, return K1, otherwise K2 
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

        template <bool IsTransparent, class K1, class K2>
        using key_arg = typename key_arg_helper<IsTransparent>::template type<K1, K2>;
    
        // return true if Args is const T& or T&&
        template <typename T, typename... Args>
        struct emplace_helper
        {
        private:
            constexpr static auto is_same_as_key = [](){
                if constexpr (sizeof...(Args) != 1)
                    return false;
                else
                    return std::conjunction_v<std::is_same<T, std::remove_cvref_t<Args>>...>;
            }();
        public:
            constexpr static bool value = is_same_as_key;
        };
    }


}

/*

Sorted Container(Bidirectional) API:

template <typename T, typename Compare, typename Allocator, typename KeyOfValue, bool UniqueKey, ...>
class SortedContainer
{
    template <bool> ContainerIterator;
    constexpr static bool IsTransparent = leviathan::collections::detail::is_transparent<Compare>;
    using Key = typename KeyOfValue::template type<T>;
    template <typename U> using key_arg_t = leviathan::collections::detail::key_arg<IsTransparent, U, Key>;

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

    // Iterators
    iterator begin();
    const_iterator begin() const; 
    iterator end();
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    reverse_iterator rbegin();
    reverse_iterator rend();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;
    const_reverse_iterator rcbegin() const;
    const_reverse_iterator rcend() const;

    // Capacity
    size_type size() const; 
    bool empty() const;
    allocator_type get_allocator() const;
    size_type max_size() const;

    // Observers
    key_compare key_comp() const;
    value_compare value_comp() const;

    // Modifiers
    std::pair<iterator, bool> insert(const value_type& x);
    std::pair<iterator, bool> insert(value_type&& x);
    iterator insert(const_iterator hint, const value_type& x);
    iterator insert(const_iterator hint, value_type& x);
    template <typename InputIterator> void insert(InputIterator first, InputIterator last);

    void insert( std::initializer_list<value_type> ilist);
    insert_return_type insert(tree_node&& nh);
    iterator insert(const_iterator hint, tree_node&& nh);

    template <typename... Args> std::pair<iterator, bool> emplace(Args&&... args);
    template <typename... Args> iterator emplace_hint(const_iterator, Args&&... args);
    void clear();

    iterator erase(const_iterator pos) 
    iterator erase(iterator pos)
    iterator erase(iterator first, iterator last)
    iterator erase(const_iterator first, const_iterator last)
    size_type erase(const key_type& x)
    template <typename K> requires (IsTransparent) size_type erase(K&& x) 
    node_type extract(const_iterator position);
    template <class K> node_type extract(K &&x);
    void merge(); 

    // Lookup
    template <typename K = key_type> size_type count(const key_arg_t<K>& x)
    template <typename K = key_type> bool contains(const key_arg_t<K>& x) const
    
    template <typename K = key_type> iterator find(const key_arg_t<K>& x)
    template <typename K = key_type> const_iterator find(const key_arg_t<K>& x) const
    template <typename K = key_type> iterator lower_bound(const key_arg_t<K>& x)
    template <typename K = key_type> const_iterator lower_bound(const key_arg_t<K>& x) const
    template <typename K = key_type> iterator upper_bound(const key_arg_t<K>& x)
    template <typename K = key_type> const_iterator upper_bound(const key_arg_t<K>& x) const
    template <typename K = key_type> std::pair<iterator, iterator> equal_range(const key_arg_t<K>& x)
    template <typename K = key_type> std::pair<const_iterator, const_iterator> equal_range(const key_arg_t<K>& x) const

};


*/


