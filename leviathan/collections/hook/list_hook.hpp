#pragma once

#include <leviathan/collections/core/node_interface.hpp>
#include <leviathan/collections/core/container_interface.hpp>
#include <leviathan/collections/common.hpp>

namespace cpp::collections::intrusive
{

// The follow functions are implemented by AI(Doubao).
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

struct list_hook : list_node_interface
{
    list_hook* links[2];
};

}  // namespace cpp::collections::intrusive
