#pragma once

#include "common.hpp"
#include "sequence_container_interface.hpp"

namespace leviathan::collections
{

template <typename X>
concept associative_container = sequence_container<X> && requires (X x)
{
    typename X::key_value;
    { x.key_comp() } -> std::same_as<typename X::key_compare>;
    { x.lower_bound(std::declval<typename X::key_type>()) } -> std::same_as<typename X::iterator>;
    { std::as_const(x).lower_bound(std::declval<typename X::key_type>()) } -> std::same_as<typename X::const_iterator>;

};

template <bool IsUnique> 
struct associative_container_lookup_interface
{
private:

    static_assert(IsUnique, "Don't support Multi-Key container");

    template <typename Self>
    auto compare_and_extractor(this Self&& self)
    {
        return std::make_pair(self.key_comp(), typename std::remove_cvref_t<Self>::key_value());
    }

public:

    template <typename Self, typename U, typename C = std::remove_cvref_t<Self>>
        requires associative_container<C>
    auto find(this Self&& self, U&& x)
    {
        // We assume the Compare is empty class.
        auto [compare, key_extractor] = self.compare_and_extractor();
        auto lower = self.lower_bound(x);
        return (lower == self.end() || compare(x, key_extractor(*lower))) ? self.end() : lower;
    }

    template <typename Self, typename U, typename C = std::remove_cvref_t<Self>>
        requires associative_container<C>
    bool contains(this Self&& self, U&& x)
    {
        return self.find(x) == self.end();
    }

    template <typename Self, typename U, typename C = std::remove_cvref_t<Self>>
        requires associative_container<C>
    auto count(this Self&& self, U&& x)
    {
        const auto c = self.find(x) == self.end() ? 0uz : 1uz;
        if constexpr (requires { typename C::size_type; })
        {
            return static_cast<typename C::size_type>(c);
        }
        else    
        {
            return c;
        }
    }

    template <typename Self, typename U, typename C = std::remove_cvref_t<Self>>
        requires associative_container<C>
    auto equal_range(this Self&& self, U&& x)
    {
        auto [compare, key_extractor] = self.compare_and_extractor();
        auto lower = self.lower_bound(x);
        auto upper = (lower == self.end() || compare(x, key_extractor(*lower))) ? lower : std::next(lower); 
        return std::make_pair(lower, upper);
    }

    template <typename Self, typename U, typename C = std::remove_cvref_t<Self>>
        requires associative_container<C>
    auto upper_bound(this Self&& self, U&& x)
    {
        return self.equal_range(x).second;
    }

};

struct associative_container_insertion_interface
{
    // (1)
    // We disable this overload directly since the overload(9) 
    // is equivalent to this.
    template <typename Self, typename C = std::remove_cvref_t<Self>>
        requires (!detail::transparent<typename C::key_compare>)
    auto insert(this Self& self, const typename C::value_type& x)
    {
        return self.emplace(x);
    }

    // (2)
    // We disable this overload directly since the overload(9) 
    // is equivalent to this.
    template <typename Self, typename C = std::remove_cvref_t<Self>>
        requires (!detail::transparent<typename C::key_compare>)
    auto insert(this Self& self, typename C::value_type&& x)
    {
        return self.emplace(std::move(x));
    }

    // (3)
    template <typename Self, typename C = std::remove_cvref_t<Self>>
    auto insert(this Self& self, typename C::const_iterator pos, const typename C::value_type& x)
    {
        return self.emplace_hint(pos, std::move(x));
    }

    // (4)
    template <typename Self, typename C = std::remove_cvref_t<Self>>
    auto insert(this Self& self, typename C::const_iterator pos, typename C::value_type&& x)
    {
        return self.emplace_hint(pos, std::move(x));
    }

    // (5)
    template <typename Self, std::input_iterator I, typename C = std::remove_cvref_t<Self>>
    auto insert(this Self& self, I first, I last)
    {
        for (; first != last; ++first)
        {
            self.emplace(*first);
        } 
    }

    // (6)
    template <typename Self, typename C = std::remove_cvref_t<Self>>
    auto insert(this Self& self, std::initializer_list<typename C::value_type> ilist)
    {
        self.insert(ilist.begin(), ilist.end());
    }

    // (7)
    // template <typename Self, typename C = std::remove_cvref_t<Self>>
    // auto insert(this Self& self, typename C::node_type&& nh);

    // (8)
    // template <typename Self, typename C = std::remove_cvref_t<Self>>
    // auto insert(this Self& self, typename C::const_iterator pos, typename C::node_type&& nh);

    // (9)
    template <typename Self, typename K, typename C = std::remove_cvref_t<Self>>
        requires (detail::transparent<typename C::key_compare>)
    auto insert(this Self& self, K&& k)
    {
        // std::cout << "MyTreeShouldCallThisFunction\n";
        return self.emplace((K&&) k);
    }

    // (10)
    template <typename Self, typename K, typename C = std::remove_cvref_t<Self>>
        requires (detail::transparent<typename C::key_compare>)
    auto insert(this Self& self, typename C::const_iterator pos, K&& k)
    {
        return self.emplace_hint(pos, (K&&) k);
    }

    template <typename Self, typename R, typename C = std::remove_cvref_t<Self>>
        requires container_compatible_range<R, typename C::value_type>
    void insert_range(this Self&& self, R&& rg)
    {
        self.insert(std::ranges::begin(rg), std::ranges::end(rg));
    }

};

struct unique_associative_container_indexer_interface
{
    // Since try_emplace will provide overload for is_transparent, so we just need pass the
    template <typename Self, typename K, typename C = std::remove_cvref_t<Self>>
    auto& operator[](this Self&& self, K&& k)
    {
        // This may not perfect, since it is unnecessary for mapped_type constructor
        // if the container already contains k. A better way is 
        // 1. Find the location of element to be inserted or updated.
        // 2. If the element does not in container, try construct a object with 
        //  default construction.
        // In this case, a default construction may be avoid in sometimes.
        return self.try_emplace((K&&) k, typename C::mapped_type()).first->second;
    }

    template <typename Self, typename K, typename M, typename C = std::remove_cvref_t<Self>>
    auto insert_or_assign(this Self&& self, K&& k, M&& obj)
    {
        auto result = self.try_emplace((K&&) k, (M&&) obj);
        if (!result.second)
        {
            result.first->second = static_cast<typename C::mapped_type>((M&&) obj); 
        }
        // The bool component is true if the insertion took place and false if the assignment took place
        return result;
    }

    template <typename Self, typename K, typename M, typename C = std::remove_cvref_t<Self>, typename... Args>
    auto insert_or_assign(this Self&& self, typename C::const_iterator hint, K&& k, Args&&... args )
    {
        // Return iterator pointing at the element that was inserted or updated.
        // And Complexity is same as for emplace_hint.
        return self.try_emplace(hint, (K&&) k, (Args&&) args...);
    }
};

} // namespace leviathan::collections

