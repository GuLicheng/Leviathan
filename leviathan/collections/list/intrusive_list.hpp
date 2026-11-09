#pragma once

#include <leviathan/collections/core/node_interface.hpp>
#include <leviathan/collections/core/container_interface.hpp>

namespace cpp::collections::intrusive
{

struct list_member_hook : public list_node_interface
{
    std::array<list_node_interface*, 2> links;
};

// https://www.boost.org/doc/libs/latest/doc/html/doxygen/classboost_1_1intrusive_1_1list.html
template <std::meta::info MemberHook>
class list : public iterable_interface
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
    using iterator = list_node_iterator<MemberHook>;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr list() 
    {
        m_header.reset();
        m_size = 0;
    }

    constexpr size_type size() const 
    { 
        return m_size;
    }

    constexpr iterator begin()
    {
        return iterator(from_hook<MemberHook>(m_header.next()));
    }
    
    constexpr const_iterator begin() const
    {
        return iterator(from_hook<MemberHook>(m_header.next()));
    }
    
    constexpr iterator end()
    {
        return iterator(&m_header);
    }
    
    constexpr const_iterator end() const
    {
        return iterator(&m_header);
    }

    constexpr void clear();

    constexpr void push_back(reference value)
    {
        m_header.insert_after(value.[:MemberHook:]);
    }

    constexpr void push_front(reference value)
    {
        m_header.insert_before(value.[:MemberHook:]);
    }

    constexpr void pop_back()
    {
        m_header.prev()->unlink();
    }

    constexpr void pop_front()
    {   
        m_header.next()->unlink();
    }

    // Erases the last element of the list. No destructors are called. 
    // Disposer::operator()(pointer) is called for the removed element.
    template <typename Disposer> 
    constexpr void pop_back_and_dispose(Disposer disposer)
    {
        auto last = m_header.prev();
        last->unlink();
        disposer(last);
    }

    // Erases the first element of the list. No destructors are called. 
    // Disposer::operator()(pointer) is called for the removed element.
    template <typename Disposer> 
    constexpr void pop_front_and_dispose(Disposer disposer)
    {
        auto first = m_header.next();
        first->unlink();
        disposer(first);
    }

private:

    list_member_hook m_header;
    size_type m_size;

};

}  // namespace cpp::collections::intrusive
