#pragma once

#include <leviathan/collections/hook/pairing_hook.hpp>

namespace cpp::collections
{

template <std::meta::info MemberHook, typename Compare = std::less<>>
class pairing_heap
{

public:

    using hook_type = typename [:type_of(MemberHook):];
    using value_type = typename [:parent_of(MemberHook):];
    using pointer = value_type*;
    using reference = value_type&;
    using const_pointer = const value_type*;
    using const_reference = const value_type&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr pairing_heap() : pairing_heap(Compare()) {}
    
    constexpr pairing_heap(const Compare& comp) : m_root(nullptr), m_comp(comp) { }



private:

    constexpr void meld(MemberHook* a, MemberHook* b)
    {
        if (!a || !b) return;

        const auto& x = from_hook<value_type>(a);
        const auto& y = from_hook<value_type>(b);

        if (m_comp(x, y))
        {
            std::swap(a, b);
        }

        b->sibling(a->child());

        if (a->child()) a->child()->sibling(b);

        a->child(b);
        b->parent(a);
    }

    MemberHook* m_root;
    Compare m_comp;

};



}  // namespace cpp::collections