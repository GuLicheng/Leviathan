#pragma once

#include <leviathan/collections/hook/list_hook.hpp>

namespace cpp::collections::intrusive
{

template <std::meta::info MemberHook>
struct list_hook_iterator
{
    using hook_type = typename [:type_of(MemberHook):];
    using value_type = typename [:parent_of(MemberHook):];
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    hook_type* current;

    constexpr list_hook_iterator() = default;
    constexpr list_hook_iterator(const list_hook_iterator&) = default;
    constexpr list_hook_iterator(hook_type* node) : current(node) { }

    constexpr reference operator*() const 
    { 
        return *from_hook<MemberHook>(current); 
    }
    
    constexpr pointer operator->() const
    {
        return from_hook<MemberHook>(current);
    }
    
    constexpr list_hook_iterator& operator++()
    {
        current = current->next();
        return *this;
    }

    constexpr list_hook_iterator operator++(int)
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    constexpr list_hook_iterator& operator--()
    {
        current = current->prev();
        return *this;
    }

    constexpr list_hook_iterator operator--(int)
    {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    constexpr bool operator==(const list_hook_iterator&) const = default;
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
    using iterator = list_hook_iterator<MemberHook>;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /**
     * @brief Constructs an empty intrusive list.
     */
    constexpr list() 
    {
        m_anchor.reset();
        m_size = 0;
    }

    constexpr list(const list&) = delete("Copy constructor is deleted for intrusive list.");
    constexpr list& operator=(const list&) = delete("Copy assignment is deleted for intrusive list.");

    /**
     * @brief Constructs a container moving resources from another container. 
     */
    constexpr list(list&& x)
    {
        m_anchor = x.m_anchor;
        m_size = x.m_size;
        x.m_anchor.reset();
        x.m_size = 0;
    }

    /**
     * @brief Equivalent to swap.
     */
    constexpr list& operator=(list&& x)
    {
        swap(x);
        return *this;
    }

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
        swap(m_anchor, other.m_anchor);
        swap(m_size, other.m_size);
    }

    constexpr size_type size() const 
    { 
        return m_size;
    }

    template <typename Self>
    constexpr self_iter_t<Self> begin(this Self&& self)
    {
        return iterator(self.m_anchor.next());
    }
    
    template <typename Self>
    constexpr self_iter_t<Self> end(this Self&& self)
    {
        return iterator(&self.m_anchor);
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
    template <typename StrictWeakOrdering>
    constexpr void sort(StrictWeakOrdering comp)
    {
        auto hook_comp = [&](auto x, auto y) {
            return comp(*iterator(x), *iterator(y));
        };
        auto node = detail::merge_sort(m_anchor.next(), &m_anchor, hook_comp);
        m_anchor.next(node);
        node->prev(&m_anchor);
    }

    /**
     * @brief Removes all elements from the list. No destructors are called.
     */
    constexpr void clear()
    {
        for (auto p = m_anchor.next(); p != &m_anchor; )
        {
            auto next = p->next();
            p->unlink();
            p = next;
        }

        m_size = 0;
        m_anchor.reset();
    }

    /**
     * @brief Removes all elements from the list and calls the provided disposer for each element.
     * Disposer::operator()(pointer) shouldn't throw.
     */
    template <typename Disposer>
    constexpr void clear_and_dispose(Disposer disposer)
    {
        for (auto p = m_anchor.next(); p != &m_anchor; )
        {
            auto next = p->next();
            disposer(p);
            p = next;
        }

        m_size = 0;
        m_anchor.reset();
    }

    /**
     * @brief Inserts the value in the back of the list. No copy constructors are called.
     * value must be an lvalue.
     */
    constexpr void push_back(reference value)
    {
        auto& hook = value.[:MemberHook:];
        hook.link(true, &m_anchor);
        m_size++;
    }

    /**
     * @brief Inserts the value in the front of the list. No copy constructors are called.
     * value must be an lvalue.
     */
    constexpr void push_front(reference value)
    {
        auto& hook = value.[:MemberHook:];
        hook.link(false, &m_anchor);
        m_size++;
    }

    /**
     * @brief Removes the last element of the list. No destructors are called.
     */
    constexpr void pop_back()
    {
        m_anchor.prev()->unlink();
        m_size--;
    }

    /**
     * @brief Removes the first element of the list. No destructors are called.
     */
    constexpr void pop_front()
    {   
        m_anchor.next()->unlink();
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
        auto last = m_anchor.prev();
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
        auto first = m_anchor.next();
        disposer(first);
        m_size--;
    }

private:
    
    list_hook m_anchor;
    size_type m_size;

};

}  // namespace cpp::collections::intrusive
