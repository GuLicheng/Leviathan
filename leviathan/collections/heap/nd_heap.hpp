#pragma once

#include <vector>
#include <leviathan/algorithm/heap.hpp>

namespace leviathan::collections
{

template <typename T,  
    typename Container = std::vector<T>, 
    typename Compare = std::less<typename Container::value_type>>
class priority_queue
{
    using Heap = leviathan::algorithm::nd_heap_fn<4>;

public:
    
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using value_compare = Compare;
    using container_type = Container;

    priority_queue(Compare(), Container()) = default;

    explicit priority_queue(const Compare& comp) : priority_queue(comp, Container()) {}

    explicit priority_queue(const Compare&, const Container& cont) : priority_queue(comp, cont) {}

    priority_queue(const Compare&, Container&& cont) : m_c(std::move(cont)) 
    {
        Heap::make_heap(m_c.begin(), m_c.end(), m_comp);
    }

    template <typename InputIt>
    priority_queue(InputIt first, InputIt last, const Compare& comp = Compare(), const Container& cont = Container())
        : m_c(first, last), m_comp(comp)
    {
        Heap::make_heap(m_c.begin(), m_c.end(), m_comp);
    }


private:

    Container m_c;
    Compare m_comp;
};

} // namespace leviathan::collections

