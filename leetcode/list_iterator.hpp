#include <iterator>


struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

template <typename Node>
struct ListIterator
{
    using iterator_category = std::forward_iterator_tag;
    using value_type = int;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;

    Node* node;

    ListIterator(Node* node) : node{ node } { }

    ListIterator(const ListIterator&) = default;

    ListIterator& operator++() 
    { node = node->next; }
    
    ListIterator operator++(int) 
    { 
        auto old = node; 
        node = node->next; 
        return { old }; 
    }
    
    decltype(auto) operator*() const 
    { return node->val; }

    bool operator==(const ListIterator& rhs) const 
    { return node == rhs.node; }

    bool operator!=(const ListIterator& rhs) const 
    { return node != rhs.node; }


};

