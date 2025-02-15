#ifndef SEGMENT_TREE_HPP
#define SEGMENT_TREE_HPP

#include <cstdint> // std::size_t
#include <type_traits> // for std::is_convertible_v

namespace leviathan::collections
{
    template <typename T>
    class segment_tree
    {
        struct node
        {
            size_t l;
            size_t r;
            T val;
            T lazy;
            node* left;
            node* right;
            node(size_t l, size_t r, T val = T{}, T lazy = T{}, node* left = nullptr, node* right = nullptr)
                : l{l}, r{r}, val{val}, lazy{lazy}, left{left}, right{right} { }
        };
        
    public:

        using value_type = T;

        template <typename Container>
        segment_tree(const Container& container) : root{nullptr} 
        {
            static_assert(std::is_convertible_v<typename Container::value_type, value_type>);
            // static_assert(std::is_same_v<typename Container::value_type, value_type>);
            if (std::size(container))
                this->build_tree(container, this->root, 0, std::size(container) - 1);
        }

        // useless
        segment_tree(const segment_tree& rhs) = delete;
        segment_tree(segment_tree&& rhs) noexcept = delete;
        segment_tree& operator=(const segment_tree& rhs) = delete;
        segment_tree& operator=(segment_tree&& rhs) noexcept = delete;
        ~segment_tree() 
        { this->clear(this->root); }

        
        // left and right must be legal or this routine will be undefined
        value_type query(const size_t left, const size_t right) 
        {
            return query(this->root, left, right);
        }


        // for each element in [left, right], element += delta
        // left and right must be leagal or this routine will be undefined
        void update(const size_t left, const size_t right, const T delta)
        {
            return update(this->root, left, right, delta);
        }



    private:

        void update(node* root, const size_t left, const size_t right, const T delta)
        {
            if (root->l > right || root->r < left)
                return;
            if (left <= root->l && root->r <= right)
            {
                apply(root, delta);
            }
            else
            {
                push(root);
                update(root->left, left, right, delta);
                update(root->right, left, right, delta);
                pull(root);
            }
            
        }

        void apply(node* root, const T delta)
        {
            root->val += (root->r - root->l + 1) * delta;
            root->lazy += delta;
        }

        void push(node* root)
        {
            const auto lazy = root->lazy;
            if (lazy)
            {
                apply(root->left, lazy);
                apply(root->right, lazy);
            }
            root->lazy = 0;
        }

        void pull(node* root)
        {
            root->val = root->left->val + root->right->val;
        }

        template <typename Container>
        void build_tree(const Container& arr, node*& root, const size_t start, const size_t end)
        {
            root = new node{start, end};
            if (start == end) 
            {
                root->val = arr[start];
                return;
            }

            size_t mid = start + (end - start) / 2;
            build_tree(arr, root->left, start, mid);
            build_tree(arr, root->right, mid + 1, end);
            root->val = (root->left ? root->left->val : 0) + (root->right ? root->right->val : 0);

        }

        value_type query(node* root, const size_t left, const size_t right)
        {
            if (left > right || !root || left > root->r || right < root->l)
                return 0;
            if (root->l >= left && root->r <= right)
                return root->val;
            push(root);
            const auto left_sum = query(root->left, left, right);
            const auto right_sum = query(root->right, left, right);
            pull(root);
            return left_sum + right_sum;
        }

        void clear(node* root)
        {
            if (root)
            {
                clear(root->left);
                clear(root->right);
                delete root;
            }
        }

        node* root;
    };
} // end of namespace
#endif