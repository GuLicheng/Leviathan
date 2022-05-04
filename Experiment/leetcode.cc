#include <string_view>
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>
#include <deque>

#include <assert.h>
#include <ctype.h>

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

/*
    list: [1, 0, 1, 1, 1, 0, 0]

    1. rfind last `1`:

    list: [1, 0, 1, 1, 1, 0, 0]
                 ^
                 pos1
                      
    2. swap (pos1 - 1) and (pos1)

    list: [1, 1, 0, 1, 1, 0, 0]
              ^  ^

    3: right shift rest 1 to end

    list: [1, 1, 0, 0, 0, 1, 1]
                             ^

*/
template <typename I, typename Comp = std::less<>>
constexpr bool combination(I first1, I middle, I last2, Comp comp = {})
{

    auto last1 = middle, first2 = middle;

    if (first1 == last1 || first2 == last2)
          return false;

    I m1 = last1;
    I m2 = last2;
    --m2;

    // find first element less than last element
    while (--m1 != first1 && !comp(*m1, *m2));

    bool result = m1 == first1 && !comp(*first1, *m2);
    if (!result)
    {
        while (first2 != m2 && !comp(*m1, *first2))
               ++first2;

        first1 = m1;
        std::iter_swap(first1, first2);
        ++first1;
        ++first2;
    }

    if (first1 != last1 && first2 != last2)
    {
        m1 = last1; 
        m2 = first2;

        while (m1 != first1 && m2 != last2)
        {
               std::iter_swap(--m1 , m2);
               ++m2;
        }

       std::reverse(first1, m1);
       std::reverse(first1, last1);
       std::reverse(m2, last2);
       std::reverse(first2, last2);
    }
    return !result;
}

template <typename I>
constexpr bool next_combination(I first, I middle, I last)
{
    return combination(first, middle, last);
}

template <typename I>
constexpr bool prev_combination(I first, I middle, I last)
{
    return combination(first, middle, last, std::greater<>());
}

#include <iostream>
#include <ranges>
#include <list>

static_assert(std::input_iterator<TreeNodeIterator>);

int main()
{

    std::list vec{ 1, 2, 2, 3 };

    int n = 2;

    do {
        std::copy_n(vec.begin(), n, std::ostream_iterator<int>{ std::cout, " " });
        std::cout << "\n======================================\n";
    } while (next_combination(vec.begin(), std::next(vec.begin(), n), vec.end()));

    std::cout << "======================================\n";

}