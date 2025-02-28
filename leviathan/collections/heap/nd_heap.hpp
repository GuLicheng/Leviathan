// https://en.cppreference.com/w/cpp/container/priority_queue

#pragma once

#include <vector>
#include <leviathan/algorithm/heap.hpp>
#include "../common.hpp"

namespace leviathan::collections
{

template <typename T,  
    typename Container = std::vector<T>, 
    typename Compare = std::less<typename Container::value_type>, 
    typename HeapFunction = leviathan::algorithm::nd_heap_fn<4>>
class priority_queue
{

public:
    
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using value_compare = Compare;
    using container_type = Container;

    priority_queue() : priority_queue(Compare(), Container()) { } 

    explicit priority_queue(const Compare& comp) : priority_queue(comp, Container()) {}

    explicit priority_queue(const Compare&, const Container& cont) 
    {
        HeapFunction::make_heap(m_container, m_comp);
    }

    priority_queue(const Compare& comp, Container&& cont) : m_comp(comp), m_container(std::move(cont)) 
    {
        HeapFunction::make_heap(m_container, m_comp);
    }

    priority_queue(const priority_queue& other) = default;

    priority_queue(priority_queue&& other) = default;

    size_type size() const 
    {
        return m_container.size();
    }

    bool empty() const 
    {
        return m_container.empty();
    }

    const_reference top() const
    {
        return m_container.front();
    }

    void push(const value_type& value)
    {
        m_container.push_back(value);
        HeapFunction::push_heap(m_container, m_comp);
    }

    void push(value_type&& value)
    {
        m_container.push_back(std::move(value));
        HeapFunction::push_heap(m_container, m_comp);
    }

    template <container_compatible_range<value_type> R>
    void push(R&& rg)
    {
        m_container.append_range((R&&)rg);
        HeapFunction::make_heap(m_container, m_comp);
        assert(HeapFunction::is_heap(m_container, m_comp));
    }

    template <typename... Args>
    void emplace(Args&&... args)
    {
        m_container.emplace_back(std::forward<Args>(args)...);
        HeapFunction::push_heap(m_container, m_comp);
    }

    void pop()
    {
        HeapFunction::pop_heap(m_container, m_comp);
        m_container.pop_back();
    }

    void swap(priority_queue& other) noexcept(std::is_nothrow_swappable_v<Container> && std::is_nothrow_swappable_v<Compare>)
    {
        using std::swap;
        swap(m_container, other.m_container);
        swap(m_comp, other.m_comp);
    }

private:

    Container m_container;
    Compare m_comp;
};

} // namespace leviathan::collections

