#pragma once

#include <type_traits>
#include <utility>
#include <memory>
#include <tuple>        
#include <optional>
#include <ranges>
#include <concepts>

#include <assert.h>

namespace leviathan::collections
{

/**
 * @brief A helper meta for set-container
 * 
 * Since set/map may implemented by same data structure, so we can 
 * add another template parameter to make the data structure set or map.
*/
template <typename T>
struct identity
{
    using key_type = T;

    using value_type = T;

    // For set<T>, the key_type is value_type.
    template <typename U>
    static constexpr auto&& operator()(U&& x)
    {
        return (U&&)x;
    }
};

/**
 * @brief A helper meta for map-container
 * 
 * Since set/map may implemented by same data structure, so we can 
 * add another template parameter to make the data structure set or map.
*/
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

namespace detail
{
// C++17 version
// template <typename T, typename = void>
// struct is_transparent : std::false_type { };

// template <typename T>
// struct is_transparent<T, std::void_t<typename T::is_transparent>> : std::true_type { };

// C++20 simply use concept and require statement
template <typename T>
concept has_transparent = requires { typename T::is_transparent; };

// Require sizeof...(Ts) > 0
template <typename... Ts>
concept transparent = (has_transparent<Ts> && ...); 

/**
 * @brief: A helper meta use non-deduced contexts to change function overload.
 *
 * https://en.cppreference.com/w/cpp/language/template_argument_deduction
 * There are some overloads in lookup member functions.
 * E.g.
 * iterator find(const key_type& key);
 * template <typename K> iterator find(const K& key);
 * This meta helper can reduce the number from 2 to 1.
 *
 * template <typename U> using key_arg_t = detail::key_arg<IsTransparent, U, key_type>;
 * template <typename K = key_type> iterator find(const key_arg_t<K>& x);
 *
 * @return: K1 if IsTransparent is true, otherwise K2.
*/
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

/**
 * @brief: A helper meta for optimizing emplace function in some containers.
 *
 * For some non-duplicated containers such as std::set, the emplace may first
 * construct a new value. However, if there is only one argument and the type
 * is const T&/T&/const T&&/T&&(where T model value_type), this step is not necessary. 
 * To avoid unnecessary copy or move operations. We use this meta helper to check 
 * whether the arguments passed in are satisfied above cases.
 *
 * @return True if std::remove_cvref_t<Args> is same as T.
*/
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

} // namespace detail

namespace detail
{
/**
 * @brief Allocate memory using allocator.
 *
 * Automatically rebind Alloc to Alloc<T>.
 * @param T Target type.
 * @param alloc Allocator.
 * @param n number of elements.
 * @return Pointer of allocator rebound to T.
 * @exception Any exception that Alloc will throw.
*/
template <typename T, typename Alloc>
auto allocate(Alloc& alloc, std::size_t n)
{
    using alloc_traits = typename std::allocator_traits<Alloc>::template rebind_traits<T>;
    using alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
    alloc_type a(alloc);
    return alloc_traits::allocate(a, n);
}

/**
 * @brief Deallocate memory using allocator.
 *
 * Automatically rebind Alloc to Alloc<T>.
 * @param alloc Allocator.
 * @param p Allocator::pointer that point the address to be deallocated.
 * @param n number of elements.
*/
template <typename Alloc, typename Pointer>
void deallocate(Alloc& alloc, Pointer p, std::size_t n)
{
    using value_type = typename std::pointer_traits<Pointer>::element_type;
    assert(p != nullptr && "p should not be nullptr");
    using alloc_traits = typename std::allocator_traits<Alloc>::template rebind_traits<value_type>;
    using alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<value_type>;
    alloc_type a(alloc);
    alloc_traits::deallocate(a, p, n);
}

/**
 * @brief Return allocator rebind with T.
 *
 * @param T Target value_type.
 * @param alloc Resource allocator.
 *
 * @return New allocator rebound with T from std::allocator_traits.
*/
template <typename T, typename Alloc>
auto rebind_allocator(Alloc& alloc)
{
    typename std::allocator_traits<Alloc>::template rebind_alloc<T> target(alloc);
    return target;
}

}  // namespace detail

/**
 * @brief A helper class use allocator construct value in construction
 *  and destroy in destruction.
 *
 * Sometimes we have to construct the value with args... first such as
 * 1. std::set::emplace since we need compare with a complete value.
 * 2. std::vector::emplace/emplace_back since the arg may alias elements of container.
 *
 * E.g.
 *  template <typename... Args>
 *  void emplace(Args&&... args) {
 *      value_handle<T, Alloc> handle(alloc, (Args&&)args...);
 *      insert(*handle);
 *  }
 * 
 * @param T value type
 * @param Allocator allocator type
*/
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

/**
 * @brief A help class to generate map container's value_compare
 * 
 * @param Pair typename Container::value_type
 * @param Compare typename Container::key_compare
 * 
 * E.g.
 *     struct value_compare : ordered_map_container_value_compare<value_type, Compare> 
 *     { 
 *     protected:
 *         friend class MapContainer; 
 *         value_compare(Compare compare) : ordered_map_container_value_compare<value_type, Compare>(compare) { }
 *     };
 * 
 * https://en.cppreference.com/w/cpp/named_req/Compare
 * Maybe the Compare is always empty class?
*/
template <typename Pair, typename Compare>
struct ordered_map_container_value_compare
{
    bool operator()(const Pair& lhs, const Pair& rhs) const
    {
        return m_c(lhs.first, rhs.first);
    }

    ordered_map_container_value_compare(Compare compare) : m_c(compare) { }

    [[no_unique_address]] Compare m_c;
};

/**
 * @brief A helper class for combining GetHashCode and Equals for hashtable.
 * 
 * @param Hasher hash function, accept one object and return its hash code.
 * @param KeyEqual equal function, check whether two objects are equal.
 * 
 * https://stackoverflow.com/questions/371328/why-is-it-important-to-override-gethashcode-when-equals-method-is-overridden
 * In some programming languages such as C#, we may need both overload
 * GetHashCode and Equals since it usually store a reference.
 * 
 * Please make sure for KeyEqual(x, y) == true, Hasher(x) == Hasher(y).
 *     
*/
template <typename Hasher, typename KeyEqual>
struct hash_key_equal : public Hasher, public KeyEqual
{
    using Hasher::operator();
    using KeyEqual::operator();

    explicit hash_key_equal(const Hasher& hasher = Hasher(), const KeyEqual& ke = KeyEqual())
         : Hasher(hasher), KeyEqual(ke) { }
};

template <typename T> 
struct cache_hash_code : std::true_type { };

// https://en.cppreference.com/w/cpp/ranges/to
template <typename R, typename T>
concept container_compatible_range = 
    std::ranges::input_range<R> &&
    std::convertible_to<std::ranges::range_reference_t<R>, T>;

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

    template <typename Self>
    auto&& value(this Self&& self)
    {
        return ((Self&&)self).m_value;
    }
};

}  // namespace leviathan::collections
