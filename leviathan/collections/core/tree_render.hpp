#pragma once

#include <string>
#include <vector>
#include <ranges>
#include <format>
#include <algorithm>

namespace cpp::collections::intrusive
{

class tree_ascii_render
{
    
public:

    template <typename Node, typename ToString>
    static constexpr std::string render_topdown(const Node* root, ToString to_string)
    {
        if (!root)
        {
            return "<empty tree>";
        }
        auto [lines, _, __] = render_topdown_impl(root, to_string);
        return lines | std::views::join_with('\n') | std::ranges::to<std::string>();
    }

    template <typename Node, typename ToString>
    static constexpr std::string render_leftright(const Node* root, ToString to_string)
    {
        if (!root)
        {
            return "<empty tree>";
        }
        std::string result;
        render_leftright_impl(root, "", true, result, to_string);
        return result;
    }

private:

    template <typename Node, typename ToString>
    static constexpr std::tuple<std::vector<std::string>, int, int> render_topdown_impl(const Node* node, ToString to_string)
    {
        if (!node)
        {
            return std::make_tuple(std::vector<std::string>(), 0, 0);
        }

        auto label = to_string(node);
        auto [left_lines, left_pos, left_width] = render_topdown_impl(node->lchild(), to_string);
        auto [right_lines, right_pos, right_width] = render_topdown_impl(node->rchild(), to_string);
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
            label += '_';
        }

        label = center(label, middle, '_');

        if (label.front() == '_')
        {
            label.front() = ' ';
        }

        if (label.back() == '_')
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
    static constexpr std::string center(const std::string& text, int width, char filler_character = ' ')
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

    // https://stackoverflow.com/questions/36802354/print-binary-tree-in-a-pretty-way-using-c
    template <typename Node, typename ToString>
    static constexpr void render_leftright_impl(const Node* node, std::string prefix, bool is_left, std::string& result, ToString to_string)
    {
        if (node)
        {
            // (│   ,├──,└──,    )
            result += std::format("{}{}{}\n", prefix, (is_left ? "|--" : "L--"), to_string(node));
            render_leftright_impl(node->lchild(), prefix + (is_left ? "|   " : "    "), true, result, to_string);
            render_leftright_impl(node->rchild(), prefix + (is_left ? "|   " : "    "), false, result, to_string);
        }
    }
};

class tree_mermaid_render
{

    template <typename Node, typename ToString>
    static constexpr void render_mermaid_impl(const Node* node, std::string& result, ToString to_string)
    {
        if (!node)
        {
            return;
        }
        auto addr = reinterpret_cast<uintptr_t>(node);
        result += std::format("{}[\"{}\"]\n", addr, to_string(node));
        if (node->lchild())
        {
            auto left = reinterpret_cast<uintptr_t>(node->lchild());
            result += std::format("{} --> {}\n", addr, left);
        }
        if (node->rchild())
        {
            auto right = reinterpret_cast<uintptr_t>(node->rchild());
            result += std::format("{} --> {}\n", addr, right);
        }
        render_mermaid_impl(node->lchild(), result, to_string);
        render_mermaid_impl(node->rchild(), result, to_string);
    }

public:

    template <typename Node, typename ToString>
    static constexpr std::string render_mermaid(const Node* root, ToString to_string)
    {
        std::string result = "```mermaid\nflowchart TD\n";
        int id = 0;
        render_mermaid_impl(root, result, to_string);
        result += "```";
        return result;
    }

};

}  // namespace cpp::collections::intrusive
