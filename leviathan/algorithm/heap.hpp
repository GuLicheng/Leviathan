// The implementation is based on the C# PriorityQueue class from .NET 6.0
// https://github.com/dotnet/runtime/blob/main/src/libraries/System.Collections/src/System/Collections/Generic/PriorityQueue.cs

#pragma once

#include "common.hpp"
#include <cmath>

namespace leviathan::algorithm
{

/**
 * @brief Some heap functions, Represents a max-heap for std::ranges::less
 * 
 * @param Arity Specifies the arity of the d-ary heap
 */
template <size_t Arity>
struct nd_heap_fn
{
    static_assert(Arity > 1 && std::has_single_bit(Arity), "Arity must be power of 2");

    // static constexpr size_t log2_arity = std::log2(Arity);
    static constexpr size_t log2_arity = std::countr_zero(Arity);

    template <typename I, typename S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr I make_heap(I first, S last, Comp comp = {}, Proj proj = {}) 
    {
        auto comp_proj = detail::make_comp_proj(comp, proj); 
        return make_heap_impl(std::move(first), std::ranges::next(first, last), comp_proj);        
    }

    template <std::ranges::random_access_range R, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
    static constexpr std::ranges::borrowed_iterator_t<R> make_heap(R&& r, Comp comp = {}, Proj proj = {}) 
    {
        return make_heap(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }

    template <typename I, typename S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr I push_heap(I first, S last, Comp comp = {}, Proj proj = {}) 
    {
        auto comp_proj = detail::make_comp_proj(comp, proj); 
        return push_heap_impl(std::move(first), std::ranges::next(first, last), comp_proj);        
    }

    template <std::ranges::random_access_range R, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
    static constexpr std::ranges::borrowed_iterator_t<R> push_heap(R&& r, Comp comp = {}, Proj proj = {}) 
    {
        return push_heap(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }

    template <typename I, typename S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr I pop_heap(I first, S last, Comp comp = {}, Proj proj = {}) 
    {
        auto comp_proj = detail::make_comp_proj(comp, proj); 
        return pop_heap_impl(std::move(first), std::ranges::next(first, last), comp_proj);        
    }

    template <std::ranges::random_access_range R, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
    static constexpr std::ranges::borrowed_iterator_t<R> pop_heap(R&& r, Comp comp = {}, Proj proj = {}) 
    {
        return pop_heap(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }

    template <typename I, typename S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr I is_heap_until(I first, S last, Comp comp = {}, Proj proj = {}) 
    {
        auto comp_proj = detail::make_comp_proj(comp, proj); 
        return is_heap_until_impl(std::move(first), 
                                  std::ranges::next(first, last), 
                                  comp_proj);        
    }

    template <std::ranges::random_access_range R, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
    static constexpr std::ranges::borrowed_iterator_t<R> is_heap_until(R&& r, Comp comp = {}, Proj proj = {}) 
    {
        return is_heap_until(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }

    template <typename I, typename S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr bool is_heap(I first, S last, Comp comp = {}, Proj proj = {}) 
    {
        return is_heap_until(std::move(first), 
                             std::ranges::next(first, last), 
                             std::move(comp), std::move(proj)) == last;
    }

    template <std::ranges::random_access_range R, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
    static constexpr bool is_heap(R&& r, Comp comp = {}, Proj proj = {}) 
    {
        return is_heap(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }

    template <std::random_access_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr I sort_heap(I first, S last, Comp comp = {}, Proj proj = {})
    {
        auto ret = std::ranges::next(first, last);
        auto comp_proj = detail::make_comp_proj(comp, proj);

        for (; first != last; --last)
        {
            pop_heap_impl(first, last, comp_proj);
        }

        return ret;
    }

    template <std::ranges::random_access_range R, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
    static constexpr std::ranges::borrowed_iterator_t<R> sort_heap(R&& r, Comp comp = {}, Proj proj = {}) 
    {
        return sort_heap(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }

private:

    // Helpers
    template <typename RandomAccessIterator, typename Comp>
    static constexpr RandomAccessIterator make_heap_impl(RandomAccessIterator first, RandomAccessIterator last, Comp& comp)
    {
        using DifferenceType = std::iter_difference_t<RandomAccessIterator>;
        using ValueType = std::iter_value_t<RandomAccessIterator>;

        DifferenceType size = std::distance(first, last);
        
        if (size < 2)
        {
            return last;  // Only one node. It is already a heap.
        }
        
        for (DifferenceType index = (size - 1) >> log2_arity; index >= DifferenceType(0); --index)
        {
            DifferenceType hold_index = index;
            ValueType hold_value = std::move(first[hold_index]);
            DifferenceType i;

            while ((i = (hold_index << log2_arity) + 1) < size)
            {
                RandomAccessIterator lower = first + i;
                RandomAccessIterator upper = first + std::min(i + DifferenceType(Arity), size);
                RandomAccessIterator max_child = std::max_element(lower, upper, std::ref(comp));

                if (!comp(hold_value, *max_child))
                {
                    break;
                }

                first[hold_index] = std::move(*max_child);
                hold_index = std::distance(first, max_child);
            }

            first[hold_index] = std::move(hold_value);
        }

        return last;
    }

    template <typename RandomAccessIterator, typename Comp>
    static constexpr RandomAccessIterator push_heap_impl(RandomAccessIterator first, RandomAccessIterator last, Comp& comp)
    {
        using DifferenceType = std::iter_difference_t<RandomAccessIterator>;
        using ValueType = std::iter_value_t<RandomAccessIterator>;

        DifferenceType size = std::distance(first, last);

        if (size < 2)
        {
            return last;  // Empty or only one root.
        }

        DifferenceType hold_index = size - 1;
        DifferenceType parent_index = (hold_index - 1) >> log2_arity;
        ValueType hold_value = std::move(first[hold_index]);

        // The parent < hold
        while (hold_index != 0 && comp(first[parent_index], hold_value)) 
        {
            first[hold_index] = std::move(first[parent_index]);
            hold_index = parent_index;
            parent_index = (hold_index - 1) >> log2_arity;
        }
        
        first[hold_index] = std::move(hold_value);
        return last;
    }

    template <typename RandomAccessIterator, typename Comp>
    static constexpr RandomAccessIterator pop_heap_impl(RandomAccessIterator first, RandomAccessIterator last, Comp& comp)
    {
        using DifferenceType = std::iter_difference_t<RandomAccessIterator>;
        using ValueType = std::iter_value_t<RandomAccessIterator>;

        DifferenceType size = std::distance(first, last);

        if (size < 2)
        {
            return last;  // Empty or only one root.
        }

        // Remove first element to last.
        std::iter_swap(first, last - 1);
        size--;

        DifferenceType hold_index = 0;
        ValueType hold_value = std::move(*first);
        DifferenceType lower_child_index;

        // Not leaf
        while ((lower_child_index = (hold_index << log2_arity) + 1) < size) 
        {
            RandomAccessIterator lower = first + lower_child_index;
            DifferenceType upper_child_index = lower_child_index + Arity;
            RandomAccessIterator upper = first + std::min(upper_child_index, size);
            RandomAccessIterator max_child = std::max_element(lower, upper, std::ref(comp));

            if (!comp(hold_value, *max_child))
            {
                // The hold is greater equal than all children.
                break;
            }

            first[hold_index] = std::move(*max_child);
            hold_index = std::distance(first, max_child);
        }

        first[hold_index] = std::move(hold_value);
        return last;
    }

    template <typename RandomAccessIterator, typename Comp>
    static constexpr RandomAccessIterator is_heap_until_impl(RandomAccessIterator first, RandomAccessIterator last, Comp& comp)
    {
        using DifferenceType = std::iter_difference_t<RandomAccessIterator>;

        DifferenceType size = std::distance(first, last);

        if (size < 2)
        {
            return last;  
        }

        DifferenceType parent = 0;
        DifferenceType child = 1;

        for (; child != size; ++child)
        {
            if (comp(first[parent], first[child]))
            {
                return first + child;
            }
            else if ((child % Arity) == 0)
            {
                ++parent;
            }
        }

        return first + size;
    }  
};

}  // namespace leviathan::algorithm




















