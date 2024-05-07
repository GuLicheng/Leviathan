// https://ocw.mit.edu/courses/6-006-introduction-to-algorithms-fall-2011/866988d04cb099a077e8e93f554c5b85_bstsize_r.py
// https://github.com/emirpasic/gods/blob/dbdbadc158ae6b453820b3cfb8c6cb48be4d7ddf/trees/avltree/avltree.go#L332
#pragma once

#include <tuple>
#include <vector>
#include <string>
#include <format>
#include <ranges>

namespace leviathan::collections
{

template <typename Tree> 
struct row_drawer
{
    using tree_node = typename Tree::tree_node;

    static std::string operator()(const Tree& tree) 
    {
        if (!tree.root())
        {
            return "<empty tree>";
        }

        auto [lines, _, __] = recurse(static_cast<const tree_node*>(tree.root()));
        std::string result;

        for (const auto& line : lines)
        {
            result += line;
            result.append("\n");
        }
        return result;
    }

    static std::tuple<std::vector<std::string>, int, int> recurse(const tree_node* node)
    {
        if (!node)
        {
            return std::make_tuple(std::vector<std::string>(), 0, 0);
        }

        auto label = std::format("{}", *static_cast<const tree_node*>(node)->value_ptr());
        
        auto [left_lines, left_pos, left_width] = recurse(static_cast<const tree_node*>(node->m_left));
        
        auto [right_lines, right_pos, right_width] = recurse(static_cast<const tree_node*>(node->m_right));
        
        auto middle = std::ranges::max({right_pos + left_width - left_pos + 1, 2, (int)label.size()});
    
        auto pos = left_pos + middle / 2;

        auto width = left_pos + middle + right_width - right_pos;

        while (left_lines.size() < right_lines.size())
        {
            left_lines.emplace_back(std::string(left_width, ' '));
        }

        while (right_lines.size() < left_lines.size())
        {
            right_lines.emplace_back(std::string(right_width, ' '));
        }

        if (((middle - label.size()) / 2 == 1)
            && (node->m_parent && node->m_parent->m_left == node)
            && (label.size() < middle))
        {
            label += '.';
        }

        label = center(label, middle, '.');

        if (label.front() == '.')
        {
            label.front() == ' ';
        }

        if (label.back() == '.')
        {
            label.back() == ' ';
        }

        // Construct the list of lines.
        char left_branch = node->m_left ? '/' : ' ';
        char right_branch = node->m_right ? '\\' : ' ';

        std::vector<std::string> lines = 
        {
            // 0
            (std::string(left_pos, ' ') + label + std::string(right_width - right_pos, ' ')), 

            // 1  
            (std::string(left_pos, ' ') + left_branch + std::string(middle - 2, ' ') + right_branch + std::string(right_width - right_pos, ' '))
        };

        // Add the right lines and left lines to the final list of lines.
        auto rg = std::views::zip_transform([=](const auto& x, const auto& y)
        {
            return x + std::string(width - left_width - right_width, ' ') + y;
        }, left_lines, right_lines);

        lines.insert(lines.end(), rg.begin(), rg.end());

        return std::make_tuple(lines, pos, width);
    }

    // See python.string.center
    static std::string center(const std::string& text, int width, char filler_character = ' ')
    {
        if (text.size() >= width)
        {
            return text;
        }

        const int length = text.size();
        
        const int right = (width - length) / 2;

        const int left = width - length - right;

        return std::string(left, filler_character) + text + std::string(right, filler_character);
    }

};

template <typename Tree>
struct column_drawer
{
    using tree_node = typename Tree::tree_node;

    static std::string operator()(const Tree& tree)
    {
        if (!tree.root())
        {
            return "<empty tree>";
        }

        std::string result;

        recurse(tree.root(), "", true, result);
        return result;
    }

    static void recurse(const tree_node* node, std::string prefix, bool is_tail, std::string& result)
    {
        if (node->m_right)
        {
            auto new_prefix = prefix + 
                (is_tail ? std::string("│   ") : std::string("    "));
        
            recurse(static_cast<const tree_node*>(node->m_right), new_prefix, false, result);
        }

        result += prefix;

        result += is_tail ? "└── " : "┌── ";

        result += std::format("{}\n", *static_cast<const tree_node*>(node)->value_ptr());
    
        if (node->m_left)
        {
            auto new_prefix = prefix + 
                (is_tail ? std::string("    ") :  std::string("│   "));
            recurse(static_cast<const tree_node*>(node->m_left), new_prefix, true, result);
        }
    }
};


template <typename Tree>
std::string draw_tree(const Tree& tree)
{
    // return row_drawer<Tree>()(tree);
    return column_drawer<Tree>()(tree);
}

} // namespace leviathan::collections

