#pragma once

#include <string>
#include <vector>
#include <ranges>
#include <format>
#include <algorithm>

namespace cpp::collections::intrusive
{

struct tree_ascii_render
{
    template <typename Self, typename ToString>
    constexpr std::string render_topdown(this const Self& self, ToString to_string)
    {
        auto root = self.header()->parent();

        if (!root)
        {
            return "<empty tree>";
        }

        auto [lines, _, __] = recurse(root, to_string);
        return lines | std::views::join_with('\n') | std::ranges::to<std::string>();
    }

    template <typename Self, typename ToString>
    constexpr std::string render_leftright(this const Self& self, ToString to_string)
    {
        return "";
    }

private:

    template <typename Node, typename ToString>
    static constexpr std::tuple<std::vector<std::string>, int, int> recurse(const Node* node, ToString to_string)
    {
        if (!node)
        {
            return std::make_tuple(std::vector<std::string>(), 0, 0);
        }

        auto label = to_string(node);
        auto [left_lines, left_pos, left_width] = recurse(node->lchild(), to_string);
        auto [right_lines, right_pos, right_width] = recurse(node->rchild(), to_string);
        auto middle = std::ranges::max({ right_pos + left_width - left_pos + 1, 2, (int)label.size() });
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
            && (node->parent() && node->parent()->lchild() == node)
            && ((int)label.size() < middle))
        {
            label += '.';
        }

        label = center(label, middle, '.');

        if (label.front() == '.')
        {
            label.front() = ' ';
        }

        if (label.back() == '.')
        {
            label.back() = ' ';
        }

        // Construct the list of lines.
        char lbranch = node->lchild() ? '/' : ' ';
        char rbranch = node->rchild() ? '\\' : ' ';

        std::vector<std::string> lines =
        {
            // 0
            (std::string(left_pos, ' ') + label + std::string(right_width - right_pos, ' ')),
            // 1  
            (std::string(left_pos, ' ') + lbranch + std::string(middle - 2, ' ') + rbranch + std::string(right_width - right_pos, ' '))
        };

        // Add the right lines and left lines to the final list of lines.
        auto rg = std::views::zip_transform([=](const auto& x, const auto& y)
            {
                return x + std::string(width - left_width - right_width, ' ') + y;
            }, left_lines, right_lines);

        // lines.insert_range(lines.end(), rg);
        lines.insert(lines.end(), rg.begin(), rg.end());

        return std::make_tuple(lines, pos, width);
    }

    // See python.string.center
    static std::string center(const std::string& text, int width, char filler_character = ' ')
    {
        if ((int)text.size() >= width)
        {
            return text;
        }

        const int length = (int)text.size();
        const int right = (width - length) / 2;
        const int left = width - length - right;

        return std::string(left, filler_character) + text + std::string(right, filler_character);
    }

};

struct column_drawer
{
    template <typename Self>
    std::string draw(this const Self& self)
    {
        auto root = self.header()->parent();

        if (!root)
        {
            return "<empty tree>";
        }

        std::string result;
        recurse2<typename Self::tree_node>(root, "", true, result);
        return result;
    }

private:

    // https://stackoverflow.com/questions/36802354/print-binary-tree-in-a-pretty-way-using-c
    template <typename D, typename Node>
    static void recurse2(const Node* node, std::string prefix, bool is_left, std::string& result)
    {
        if (node)
        {
            // (│   ,├──,└──,    )
            result += std::format("{}{}{}\n", prefix, (is_left ? "|--" : "L--"), static_cast<const D*>(node)->value());

            recurse2<D>(node->lchild(), prefix + (is_left ? "|   " : "    "), true, result);
            recurse2<D>(node->rchild(), prefix + (is_left ? "|   " : "    "), false, result);
        }
    }
};



}  // namespace cpp::collections::intrusive
