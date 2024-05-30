#include "generator.hpp"
#include <iostream>

struct Node
{
    Node* left;
    Node* right;
    int val;
};

cppcoro::generator<int> Leaves(Node* node)
{
    if (not node->left and not node->right)
    {
        co_yield node->val;
    }

    if (node->left)
    {
        for (auto x : Leaves(node->left))
        {
            co_yield x;
        }
    }

    if (node->right)
    {
        for (auto x : Leaves(node->right))
        {
            co_yield x;
        }
    }

}

int main(int argc, char const *argv[])
{
    
    Node* r = new Node {
        .left = new Node {
            .left = nullptr,
            .right = nullptr,
            .val = -1
        },
        .right = new Node {
            .left = nullptr,
            .right = nullptr,
            .val = 1
        },
        .val = 0
    };

    for (auto v : Leaves(r))
    {
        std::cout << v << ' ';
    }

    return 0;
}

