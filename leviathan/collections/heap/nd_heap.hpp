
#pragma once

#include <vector>
#include <leviathan/algorithm/heap.hpp>


namespace leviathan::collections
{



// inline constexpr heapify_fn<4> heapify{};

template <typename T,  
    typename Container = std::vector<T>, 
    typename Compare = std::less<typename Container::value_type>
    size_t Arity = 4>
class nd_heap
{
    using QuaternaryMaxHeapFunction = leviathan::algorithm::nd_heap_fn<4>;


    Container m_data;
    Compare m_compare;
};

} // namespace leviathan::collections

