#include <string_view>
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>
#include <deque>

#include <assert.h>
#include <ctype.h>
#include <lv_cpp/algorithm/combination.hpp>


#include <iostream>


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

// In-order iterator
struct TreeNodeIterator 
{
    using value_type = ValueType;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;
    using pointer = value_type*;

    std::shared_ptr<std::vector<NodeType*>> m_stack = nullptr;

    TreeNodeIterator() = default;

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

    void operator++(int)
    {
        (void)++ *this;
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

struct CBTInserterIterator
{
    using value_type = void;
    using reference = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using iterator_category = std::output_iterator_tag;


    NodeType* m_root = nullptr; // for return root
    std::shared_ptr<std::deque<NodeType*>> m_queue = nullptr;

    CBTInserterIterator() = default;

    CBTInserterIterator(NodeType* root) 
        : m_root{ root }, m_queue { std::make_shared<std::deque<NodeType*>>() }
    {
        assert(root != nullptr);
        // BFS 

        std::deque<NodeType*> queue;
        queue.emplace_back(root);
        while (queue.size())
        {
            auto node = queue.front();
            queue.pop_front();
            if (node->left && node->right) 
            {
                queue.emplace_back(node->left);
                queue.emplace_back(node->right);
            }
            else if (node->left)
            {   
                // only has left child
                queue.emplace_back(node->left);
                m_queue->emplace_back(node); // this node can be father
            }
            else
            {
                // left and right is null
                m_queue->emplace_back(node);
            }
        }
    }

    CBTInserterIterator& operator*() { return *this; }

    CBTInserterIterator& operator++() { return *this; }

    CBTInserterIterator operator++(int) { return *this; }

    CBTInserterIterator& operator=(ValueType value)
    {
        
        if (!m_root) 
        {
            m_queue = std::make_shared<std::deque<NodeType*>>();
            m_queue->emplace_back(new NodeType(value));
            m_root = m_queue->front();
            return *this;
        }


        auto new_node = new NodeType(value);
        m_queue->emplace_back(new_node);


        auto node = m_queue->front();
        if (!node->left) node->left = new_node;
        else 
        {
            node->right = new_node;
            m_queue->pop_front();
        }
        return *this;
    }

};


#include <ranges>
#include <list>

static_assert(std::input_iterator<TreeNodeIterator>);


int main()
{

}