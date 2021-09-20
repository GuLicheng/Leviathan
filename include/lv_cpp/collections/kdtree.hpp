#ifndef __KDTREE_HPP__
#define __KDTREE_HPP__


#include <vector>
#include <concepts>
#include <algorithm>
#include <iostream>
#include <cmath>
/*

                (5, 0)
        (3, 5)          (6, 2)
    (1, 2)  (4, 9)  (8, 1) (6, 3)

*/

namespace leviathan::collections
{

    template <typename Range>
    void print(const Range& range)
    {
        std::cout << '[';
        auto first = std::ranges::begin(range);
        auto last = std::ranges::end(range);

        for (auto iter = first; iter != last; ++iter)
        {
            if (iter != first)
                std::cout << ", ";
            std::cout << *iter;
        }

        std::cout << ']';
    }

    template <size_t Dimension, typename Scalar, typename Compare = std::less<void>>
    class kd_tree
    {
        struct kd_node
        {
            Scalar m_feature;
            size_t m_split;
            kd_node* m_left;
            kd_node* m_right;
            kd_node* m_father;

            kd_node(const Scalar& scalar, size_t split, kd_node* left = nullptr, kd_node* right = nullptr)
                : m_feature{ scalar }, m_split{ split }, m_left{ left }, m_right{ right }, m_father{ nullptr }
            {
            }
            kd_node(Scalar&& scalar, size_t split, kd_node* left = nullptr, kd_node* right = nullptr)
                : m_feature{ std::move(scalar) }, m_split{ split }, m_left{ left }, m_right{ right }, m_father{ nullptr }
            {
            }

        };
    public:
        template <std::random_access_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
        requires (std::same_as<Scalar, std::iter_value_t<Iterator>>)
            kd_tree(Iterator first, Sentinel last, Compare compare = {})
            : m_cmp{ std::move(compare) }, m_size{ 0 }
        {
            const auto index = choose_feature(first, last);
            this->m_root = build_tree(first, last, index);
        }

        constexpr auto dimension() const
        {
            return Dimension;
        }

        void show() const
        {
            show(this->m_root);
        }

        void show(const kd_node* r) const
        {
            if (r)
            {
                show(r->m_left);
                print(r->m_feature);
                if (r->m_father)
                {
                    std::cout << "Father node is ";
                    print(r->m_father->m_feature);
                    std::cout << '\n';
                }
                show(r->m_right);
            }
        }

    private:

        kd_node* m_root;
        Compare m_cmp;
        size_t m_size;

        template <typename Iterator, typename Sentinel>
        size_t choose_feature(Iterator first, Sentinel last)
        {
            constexpr auto d = Dimension;
            using floating_type = float;
            std::vector<floating_type> means(d);
            for (size_t i = 0; i < d; ++i)
            {
                floating_type sum = 0;
                for (auto iter = first; iter != last; ++iter)
                {
                    sum += (*iter)[i];
                }
                means[i] = sum / d;
            }
            std::vector<floating_type> var(d);
            for (int i = 0; i < d; ++i)
            {
                floating_type sum = 0;
                for (auto iter = first; iter != last; ++iter)
                {
                    sum += ((*iter)[i] - means[i]) * ((*iter)[i] - means[i]);
                }
                var[i] = std::sqrt(sum);
            }
            return std::ranges::distance(var.begin(), std::ranges::max_element(var));
        }

        // Iterator's value should be a tensor or other random access range
        template <typename Iterator, typename Sentinel>
        kd_node* build_tree(Iterator first, Sentinel last, size_t dim)
        {
            if (first == last)
                return nullptr;

            const auto size = std::ranges::distance(first, last) / 2;
            auto mid = first + size;
            std::ranges::nth_element(first, first + size, last, [this, dim](const auto& left, const auto& right)
                {
                    return this->m_cmp(left[dim], right[dim]);
                });

            // build left and build right
            kd_node* node = new kd_node(std::move(*mid), dim);
            size_t d = (dim + 1) % dimension();
            node->m_left = build_tree(first, mid, d);
            if (node->m_left)
                node->m_left->m_father = node;
            node->m_right = build_tree(mid + 1, last, d);
            if (node->m_right)
                node->m_right->m_father = node;
            return node;
        }

    };


    // int main()
    // {
    //     std::vector<std::vector<int>> features =
    //     {
    //         { 6, 3 }, { 6, 2 }, { 4, 9 }, { 5, 0 }, { 1, 2 }, { 3, 5 }, { 8, 1}
    //     };

    //     kd_tree<2, std::vector<int>, std::less<void>> tree{ features.begin(), features.end() };

    //     tree.show();
    // }

} // end of namespace

#endif