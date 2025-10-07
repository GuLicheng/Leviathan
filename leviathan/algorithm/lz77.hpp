/*
    FIXME: This implementation is not optimized for performance or compression ratio.
    It is just a simple implementation for educational purposes.
*/
#pragma once

#include <iterator>
#include <vector>
#include <algorithm>
#include <ranges>
#include <format>
#include <string>
#include <type_traits>

namespace cpp::algorithm
{

template <typename I1, typename I2>
constexpr I1 lz77_compress_match(I1 first1, I1 last1, I2 first2, I2 last2)
{
    using DifferenceType = std::iter_difference_t<I1>;

    DifferenceType size1 = std::ranges::distance(first1, last1);
    DifferenceType size2 = std::ranges::distance(first2, last2);

    // 1 -> [1, 2, 3, 4]
    // 2 ->       [3, 4]
    for (DifferenceType offset = 0; offset < size1; ++offset)
    {
        DifferenceType length = 0;

        for (; length < size2 && first1[(offset + length) % size1] == first2[length]; ++length);

        if (length == size2)
        {
            return std::ranges::next(first1, offset);
        }
    }

    return last1;
} 
    
template <typename I>
struct lz77_token
{
    using value_type = std::iter_value_t<I>;
    static_assert(sizeof(value_type) == 1, "value_type must be something like char, uint8_t, std::byte etc.");

    std::iter_difference_t<I> offset;
    std::iter_difference_t<I> length;
    I next;
};

template <typename I>
constexpr std::vector<lz77_token<I>> lz77_compress(I first, I last, std::iter_difference_t<I> window_size, std::iter_difference_t<I> lookahead_size)
{
    static_assert(std::random_access_iterator<I>, "I must be a random access iterator.");
    using DifferenceType = std::iter_difference_t<I>;
    using Token = lz77_token<I>;

    std::vector<Token> result;

    auto current = first;

    while (current != last) 
    {
        auto search_start = std::ranges::prev(current, window_size, first);
        auto lookahead_end = std::ranges::next(current, lookahead_size, last);

        DifferenceType offset = 0;
        DifferenceType length = 0;
        I pos;
        auto it = std::ranges::next(current);

        for (; it != lookahead_end; ++it) 
        {
            // buffer -> [current, it)
            // std::println("window = {} buffer = {}", std::ranges::subrange(search_start, current), std::ranges::subrange(current, it));
            pos = lz77_compress_match(search_start, current, current, it);

            if (pos == current) 
            {
                break;
            }

            length++;
            offset = std::ranges::distance(pos, current);
        }

        if (it == lookahead_end) 
        {
            pos = lz77_compress_match(search_start, current, current, it);

            if (pos != current) 
            {
                length++;
                offset = std::ranges::distance(pos, current);
            }
        }

        if (length == 0) 
        {
            result.emplace_back(0, 0, current);
            current++;
        } 
        else 
        {
            result.emplace_back(offset, length, current + length);
            std::ranges::advance(current, length + 1, last);
        }

    }

    return result;
}

template <typename I>
constexpr auto lz77_decompress(const std::vector<lz77_token<I>>& tokens)
{
    using ValueType = std::iter_value_t<I>;
    using DifferenceType = std::iter_difference_t<I>;

    std::vector<ValueType> result;

    for (const auto& token : tokens) 
    {
        if (token.offset == 0 && token.length == 0) 
        {
            result.emplace_back(*token.next);
        } 
        else 
        {
            DifferenceType start = std::ranges::distance(result.begin(), result.end()) - token.offset;

            for (DifferenceType i = 0; i < token.length; ++i) 
            {
                result.emplace_back(result[start + i]);
            }

            if (token.next != I{}) 
            {
                result.emplace_back(*token.next);
            }
        }
    }

    return result;
}

template <typename T>
constexpr std::vector<lz77_token<const T*>> lz77compress(const T* first, const T* last, size_t window_size, size_t lookahead_size)
{
    std::vector<lz77_token<const T*>> result;

    auto current = first;

    while (current != last) 
    {
        auto search_start = std::ranges::prev(current, window_size, first);
        auto lookahead_end = std::ranges::next(current, lookahead_size, last);

        std::ptrdiff_t offset = 0;
        std::ptrdiff_t length = 0;
        const T* pos = nullptr;
        auto it = std::ranges::next(current);

        for (; it != lookahead_end; ++it) 
        {
            // buffer -> [current, it)
            // std::println("window = {} buffer = {}", std::ranges::subrange(search_start, current), std::ranges::subrange(current, it));
            pos = lz77_compress_match(search_start, current, current, it);

            if (pos == current) 
            {
                break;
            }

            length++;
            offset = std::ranges::distance(pos, current);
        }

        if (it == lookahead_end) 
        {
            pos = lz77_compress_match(search_start, current, current, it);

            if (pos != current) 
            {
                length++;
                offset = std::ranges::distance(pos, current);
            }
        }

        if (length == 0) 
        {
            result.emplace_back(0, 0, current);
            current++;
        } 
        else 
        {
            result.emplace_back(offset, length, current + length);
            std::ranges::advance(current, length + 1, last);
        }

    }

    return result;
}

template <typename I>
struct huffman_result
{
    struct node
    {
        I position;
        std::string code;
    };

    std::vector<node> nodes;

    void add_node(I position, std::string code)
    {
        nodes.emplace_back(position, std::move(code));
    }

    std::string encode(I first, I last) const
    {
        std::string result;

        for (auto it = first; it != last; ++it)
        {
            auto node_it = std::ranges::find_if(nodes, [&](const auto& n) { return *(n.position) == *it; });

            if (node_it != nodes.end())
            {
                result += node_it->code;
            }
            else
            {
                throw std::runtime_error("Character not found in Huffman tree.");
            }
        }

        return result;
    }

    auto show() const
    {
        for (const auto& n : nodes)
        {
            std::println("pos = {}, code = {}", *n.position, n.code);
        }
    }
};

template <typename I>
constexpr huffman_result<I> huffman(I first, I last)
{
    huffman_result<I> result;
    std::vector<std::pair<I, size_t>> freq;   // We use std::vector instead of map/unordered_map, we assume the input range is not very large.

    for (auto cur = first; cur != last; ++cur)
    {
        auto it = std::ranges::find_if(freq, [&](const auto& p) { return *(p.first) == *cur; });

        if (it == freq.end())
        {
            freq.emplace_back(cur, 1);
        }
        else
        {
            it->second++;
        }
    }

    std::ranges::sort(freq, std::ranges::greater(), &std::pair<I, size_t>::second);

    // for (const auto& [pos, count] : freq)
    for (const auto& [idx, pos_and_count] : freq | std::views::enumerate | std::views::take(freq.size() - 1))
    {
        result.add_node(pos_and_count.first, std::format("{}0", std::string(idx, '1')));
    }

    // The last one is all 1s.
    result.add_node(freq.back().first, std::string(freq.size() - 1, '1'));
    return result;
}













} // namespace cpp::algorithm

