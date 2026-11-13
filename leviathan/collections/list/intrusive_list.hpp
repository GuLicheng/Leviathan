#pragma once

#include <leviathan/collections/core/node_interface.hpp>
#include <leviathan/collections/core/container_interface.hpp>

namespace cpp::collections::intrusive
{

struct list_member_hook : public list_node_interface
{
    std::array<list_member_hook*, 2> links;
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

    /**
     * @brief Constructs an empty intrusive list.
     */
    constexpr list() 
    {
        m_header.reset();
        m_size = 0;
    }

    constexpr list(list&& x);

    constexpr list & operator=(list&& x);

    /**
     * @brief It detaches all elements from this. In this case the objects in the list 
     * are not deleted.
     */
    constexpr ~list()
    {
        clear();
    }

    void swap(list& other)
    {
        using std::swap;
        swap(m_header, other.m_header);
        swap(m_size, other.m_size);
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
        return const_cast<list*>(this)->begin();
    }
    
    constexpr iterator end()
    {
        return iterator(from_hook<MemberHook>(&m_header));
    }
    
    constexpr const_iterator end() const
    {
        return const_cast<list*>(this)->end();
    }

    /**
     * @brief This function sorts the list *this according to operator <. 
     * The sort is stable, that is, the relative order of equivalent elements is preserved.
     */
    constexpr void sort()
    {
        sort(std::ranges::less());
    }

    /**
     * @brief Sorts the elements in the list using the provided comparison function.
     * The sort is stable, that is, the relative order of equivalent elements is preserved.
     */
    template <typename Compare>
    constexpr void sort(Compare comp);

    /**
     * @brief Removes all elements from the list. No destructors are called.
     */
    constexpr void clear()
    {
        for (auto p = m_header.next(); p != &m_header; )
        {
            auto next = p->next();
            p->unlink();
            p = next;
        }

        m_size = 0;
        m_header.reset();
    }

    /**
     * @brief Removes all elements from the list and calls the provided disposer for each element.
     * Disposer::operator()(pointer) shouldn't throw.
     */
    template <typename Disposer>
    constexpr void clear_and_dispose(Disposer disposer)
    {
        for (auto p = m_header.next(); p != &m_header; )
        {
            auto next = p->next();
            disposer(p);
            p = next;
        }

        m_size = 0;
        m_header.reset();
    }

    /**
     * @brief Inserts the value in the back of the list. No copy constructors are called.
     * value must be an lvalue.
     */
    constexpr void push_back(reference value)
    {
        auto& hook = value.[:MemberHook:];
        m_header.insert_after(std::addressof(hook));
        m_size++;
    }

    /**
     * @brief Inserts the value in the front of the list. No copy constructors are called.
     * value must be an lvalue.
     */
    constexpr void push_front(reference value)
    {
        auto& hook = value.[:MemberHook:];
        m_header.insert_before(std::addressof(hook));
        m_size++;
    }

    /**
     * @brief Removes the last element of the list. No destructors are called.
     */
    constexpr void pop_back()
    {
        m_header.prev()->unlink();
        m_size--;
    }

    /**
     * @brief Removes the first element of the list. No destructors are called.
     */
    constexpr void pop_front()
    {   
        m_header.next()->unlink();
        m_size--;
    }

    /**
     * @brief Erases the last element of the list. No destructors are called. 
     * Disposer::operator()(pointer) is called for the removed element.
     * Disposer::operator()(pointer) shouldn't throw.
     */
    template <typename Disposer> 
    constexpr void pop_back_and_dispose(Disposer disposer)
    {
        auto last = m_header.prev();
        disposer(last);
        m_size--;
    }

    /**
     * @brief Erases the first element of the list. No destructors are called. 
     * Disposer::operator()(pointer) is called for the removed element.
     * Disposer::operator()(pointer) shouldn't throw.
     */
    template <typename Disposer> 
    constexpr void pop_front_and_dispose(Disposer disposer)
    {
        auto first = m_header.next();
        disposer(first);
        m_size--;
    }

private:

    list_member_hook m_header;
    size_type m_size;

};

}  // namespace cpp::collections::intrusive
