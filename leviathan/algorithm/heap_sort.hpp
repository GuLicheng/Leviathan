#pragma once

#include "basic_sort.hpp"
#include "heap.hpp"

namespace cpp::ranges
{

namespace detail
{

template <size_t Arity>
struct heap_sort_fn
{
    template <std::random_access_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) 
    {
        auto tail = std::ranges::next(first, last);
        nd_heap_fn<Arity>::heap_sort(first, tail, std::move(comp), std::move(proj));
        return tail;    
    }   

    template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity> 
        requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj> 
    constexpr std::ranges::borrowed_iterator_t<Range> static operator()(Range &&r, Comp comp = {}, Proj proj = {})  
    {
        return operator()(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }
};

}  // namespace detail

template <size_t Arity = 4>
inline constexpr detail::heap_sort_fn<Arity> nd_heap_sort;

}


