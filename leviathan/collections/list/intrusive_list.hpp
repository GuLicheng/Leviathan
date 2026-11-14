#pragma once

#include <leviathan/collections/core/node_interface.hpp>
#include <leviathan/collections/core/container_interface.hpp>
#include <leviathan/collections/common.hpp>

namespace cpp::collections::intrusive
{

namespace detail
{

// [first, last) 快慢指针找中点，分割为 [first, middle) 与 [middle, last)
template <typename ListNode>
constexpr ListNode* half(ListNode* first, ListNode* last) noexcept
{
    ListNode* slow = first;
    ListNode* fast = first;

    while (fast != last && fast->next() != last) 
    {
        slow = slow->next();
        fast = fast->next()->next();
    }
    return slow;
}

// 合并两条已排序链；两链尾均已 detach 成自环（tail->next() == tail）。
// 通过 out_tail 返回合并后的新链尾，避免上层再线性走查。
template <typename ListNode, typename Compare>
constexpr ListNode* merge(ListNode* a, ListNode* b, Compare comp, ListNode*& out_tail)
{
    ListNode dummy;                 // 局部哑节点（节点自环初始化）
    ListNode* tail = &dummy;

    bool a_done = false;
    bool b_done = false;

    while (!a_done || !b_done) 
    {
        if (!a_done && (b_done || comp(a, b))) 
        {
            ListNode* n = a->next();
            tail->next(a);
            a->prev(tail);
            tail = a;
            if (n == a) a_done = true;      // 取到自环尾，左链耗尽
            a = n;
        }
        else if (!b_done) 
        {
            ListNode* n = b->next();
            tail->next(b);
            b->prev(tail);
            tail = b;
            if (n == b) b_done = true;     // 取到自环尾，右链耗尽
            b = n;
        }
    }

    out_tail = tail;                    // 循环退出时 tail 即新链尾
    return dummy.next();
}

template <typename ListNode, typename Compare>
constexpr ListNode* merge_sort(ListNode* first, ListNode* last, Compare comp)
{
    assert(first && last && "Both first and last nodes must be non-null for merge_sort.");

    if (first == last) return first;
    if (first->next() == last) return first;       // 单元素区间，终止递归

    auto middle = half(first, last);
    auto left   = merge_sort(first, middle, comp);

    // 左递归刚 reconnect 完，middle->prev() 此刻就是左链尾；
    // 必须在右递归之前抢，否则会被右链的 merge 改写。
    ListNode* ltail = middle->prev();

    auto right = merge_sort(middle, last, comp);
    ListNode* rtail = last->prev();

    ltail->next(ltail);                 // detach 成自环，供 merge 判断链尾
    rtail->next(rtail);

    ListNode* mtail = nullptr;
    ListNode* merged = merge(left, right, comp, mtail);

    // 中间层：只把新尾接回 last，绝不动 last->next()
    // （它是父层右链的起点，动了会破坏拓扑）。
    mtail->next(last);
    last->prev(mtail);

    return merged;
}

}  // namespace detail

struct list_member_hook : public list_node_interface
{
    list_member_hook* links[2];
};

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
        m_header.reset();
        m_size = 0;
    }

    /**
     * @brief Constructs a container moving resources from another container. 
     */
    constexpr list(list&& x)
    {
        m_header = x.m_header;
        m_size = x.m_size;
        x.m_header.reset();
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
        swap(m_header, other.m_header);
        swap(m_size, other.m_size);
    }

    constexpr size_type size() const 
    { 
        return m_size;
    }

    template <typename Self>
    constexpr self_iter_t<Self> begin(this Self&& self)
    {
        return iterator(self.m_header.next());
    }
    
    template <typename Self>
    constexpr self_iter_t<Self> end(this Self&& self)
    {
        return iterator(&self.m_header);
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
        auto node = detail::merge_sort(m_header.next(), &m_header, hook_comp);
        m_header.next(node);
        node->prev(&m_header);
    }

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
