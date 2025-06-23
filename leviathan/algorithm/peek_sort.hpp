// https://arxiv.org/pdf/1805.04154

// We may implement a peek sort algorithm based on the paper "Nearly Optimal Mergesort Code"
// if some language standard library use it as built-in sort algorithm.
#if 0
#pragma once

#include "basic_sort.hpp"
#include "tim_sort.hpp"

namespace cpp::ranges::detail 
{

template <int InsertionSortThreshold = 24> 
class peek_sorter;

}

#endif