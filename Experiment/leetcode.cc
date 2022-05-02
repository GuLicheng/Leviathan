#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

using ValueType = int;
using NodeType = TreeNode;

struct TreeNodeIterator 
{
    using value_type = ValueType;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;
    using pointer = value_type*;

    std::shared_ptr<std::vector<NodeType*>> m_stack = nullptr;

    TreeNodeIterator() = default;
    TreeNodeIterator(const TreeNodeIterator&) = default;
    TreeNodeIterator& operator=(const TreeNodeIterator&) = default;
    TreeNodeIterator(TreeNodeIterator&&) noexcept = default;
    TreeNodeIterator& operator=(TreeNodeIterator&&) noexcept = default;

    TreeNodeIterator(TreeNode* node) : m_stack{ std::make_shared<std::vector<NodeType*>>() } 
    {
        while (node)
        {
            m_stack->emplace_back(node);
            node = node->left;
        }   
    }

    TreeNodeIterator& operator++()
    {
        auto node = m_stack->back();
        m_stack->pop_back();

        node = node->right;
        while (node) 
        {
            m_stack->emplace_back(node); 
            node = node->left;       
        }
        return *this;
    }

    TreeNodeIterator operator++(int)
    {
        auto old = *this;
        ++ *this;
        return old;
    }
    
    bool operator==(const TreeNodeIterator& rhs) const 
    {
        return m_stack->empty();
    }

    bool operator!=(const TreeNodeIterator& rhs) const 
    {
        return !this->operator==(rhs);
    }

    reference operator*() const
    {
        auto node = m_stack->back();
        return node->val;
    }

};

#include <iostream>
#include <lv_cpp/generator.hpp>

cppcoro::generator<int> make_binary_iterator(TreeNode* root)
{
    if (root->left) co_yield root->left->val;
    co_yield root->val;
    if (root->right) co_yield root->right->val;
}


class Solution {
public:
    std::vector<int> getAllElements(TreeNode* root1, TreeNode* root2) {
        std::vector<int> res;
    
        auto iter1 = TreeNodeIterator(root1);
        auto iter2 = TreeNodeIterator(root2);

        auto sentinel = TreeNodeIterator();

        std::merge(iter1, sentinel, iter2, sentinel, std::back_inserter(res));
        return res;
    }
};


#include <iostream>
#include <ranges>

static_assert(std::input_iterator<TreeNodeIterator>);

int main()
{

    TreeNode* left = new TreeNode(0);

    TreeNode* root = new TreeNode(1, left, nullptr);

    auto gen = make_binary_iterator(root);

    for (auto val : gen)
        std::cout << val << '\n';

    std::cout << "======================================\n";

}