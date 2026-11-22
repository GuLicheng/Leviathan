#pragma once

namespace cpp::collections
{

template <typename Node, typename Key, typename Compare>
constexpr Node* bs_lower_bound(Node* root, const Key& key, Compare comp)
{
    Node* px = nullptr;
    Node* x = root;

    while (x)
    {
        if (!comp(x, key))
        {
            px = x;
            x = x->lchild();
        }
        else
        {
            x = x->rchild();
        }
    }

    return px;
}

template <typename Node, typename Key, typename Compare>
constexpr Node* bs_upper_bound(Node* root, const Key& key, Compare comp)
{
    Node* px = nullptr;
    Node* x = root;

    while (x)
    {
        if (comp(key, x))
        {
            px = x;
            x = x->lchild();
        }
        else
        {
            x = x->rchild();
        }
    }

    return px;
}

// FIEME
template <typename Node, typename Key, typename Compare>
constexpr std::pair<Node*, Node*> bs_equal_range(Node* root, const Key& key, Compare comp)
{
    // comp(node, key)  → node < key
    // 目标：[lower, upper)
    Node* curr = root;
    Node* lower = nullptr;
    Node* upper = nullptr;

    while (curr != nullptr)
    {
        if (comp(curr, key))
        {
            // curr < key → 往右
            curr = curr->rchild();
        }
        else if (comp(key, curr))
        {
            // key < curr → 候选upper，往左
            upper = curr;
            curr = curr->lchild();
        }
        else
        {
            // curr == key：curr是lower候选，继续向左找更小的>=key
            lower = curr;
            curr = curr->lchild();
        }
    }
    return {lower, upper};
}


}  // namespace cpp::collections
