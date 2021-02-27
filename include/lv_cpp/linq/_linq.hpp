/*
    A rang-loop expression may such as:
    
    for (var iter = sequence.GetEnumerator(); iter.IsOver(); iter.Next())
        do 
            iter.Current();
        
    ->

    for (auto iter = begin(sequence); iter != end(sequence); ++iter)
        do 
            *iter;

*/

#ifndef __LINQ_HPP__
#define __LINQ_HPP__

#include <tuple>
#include <functional>
#include <unordered_set>
#include <memory>

namespace leviathan::linq
{



    enum
    {
        ITER_PAIR = 0, BEGIN, END, NEXT, PREV, DEREF, EQUAL
    };

    template <typename Storage>
    class linq
    {
        template <typename _Storage>
        friend class linq;

        Storage m_store;

        linq(Storage store) : m_store{std::move(store)}
        {
        }
    public:


        template <typename Range>
        friend auto from(Range& __r);

        template <typename Transform>
        constexpr linq for_each(Transform&& transform) const
        {
            auto store = std::get<ITER_PAIR>(this->m_store);
            auto first = std::get<BEGIN>(this->m_store)(store);
            auto last = std::get<END>(this->m_store)(store);
            auto is_over = std::get<EQUAL>(this->m_store);
            auto next = std::get<NEXT>(this->m_store);
            auto deref = std::get<DEREF>(this->m_store);
            for (auto iter = first; !is_over(iter, last); iter = next(iter))
            {
                transform(deref(iter));
            }
            return *this;
        }

    };

    constexpr auto begin = [](auto& __iter_pair) { return std::get<0>(__iter_pair); };
    constexpr auto end = [](auto& __iter_pair) { return std::get<1>(__iter_pair); };
    constexpr auto next = [](auto&& __iter) { return std::next(__iter); };
    constexpr auto prev = [](auto&& __iter) { return std::prev(__iter); };
    constexpr auto deref = [](auto&& __iter) -> decltype(auto) { return *__iter; };
    constexpr auto equal = std::equal_to<void>();

    template <typename Range>
    auto from(Range& __r)
    {
        auto iter_pair = std::make_tuple(std::begin(__r), std::end(__r));
        auto tuple = std::make_tuple(iter_pair, begin, end, next, prev, deref, equal);
        return linq<decltype(tuple)>(tuple);
    }

}


#endif