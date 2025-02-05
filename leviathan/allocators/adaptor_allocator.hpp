#pragma once

#include <memory>

namespace leviathan
{
    
/**
 * @brief An adaptor for allocator, automatically rebind Alloc to Alloc<T>.
 * 
 * @param Alloc The Allocator type of collection/container.
 * @param T The react type. 
 * 
 * 
 * E.g.
 * 
 *  struct ListNode { ... };
 *  using List = LinkList<int, std::allocator<int>>; 
 *  
 *  using adaptor = adaptor_allocator<std::allocator<int>, ListNode>;
 *  Allocate memory: adaptor::allocate(alloc, 1);
 */
template <typename Alloc, typename T>
struct adaptor_allocator
{
    using size_type = typename Alloc::size_type;
    using const_void_pointer = typename Alloc::const_void_pointer;
    using pointer = typename Alloc::pointer;
    using alloc_traits = typename std::allocator_traits<Alloc>::template rebind_traits<T>;

    constexpr static auto rebind_allocator(Alloc& alloc)
    {
        typename std::allocator_traits<Alloc>::template rebind_alloc<T> target(alloc);
        return target;
    }

    constexpr static allocate(Alloc& alloc, size_type n)
    {
        auto a = rebind_allocator(alloc);
        return alloc_traits::allocate(a, n);
    }

    constexpr static allocate(Alloc& alloc, size_type n, const_void_pointer hint)
    {
        auto a = rebind_allocator(alloc);
        return alloc_traits::allocate(a, n, hint);
    }

    constexpr static void deallocate(Alloc& a, pointer p, size_type n)
    {
        using value_type = typename std::pointer_traits<pointer>::element_type;
        assert(p != nullptr && "p should not be nullptr");
        auto a = rebind_allocator(alloc);
        alloc_traits::deallocate(a, p, n);
    }
};

} // namespace leviathan

