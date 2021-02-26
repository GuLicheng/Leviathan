/*
    A rang-loop expression may such as:
    
    for (var iter = obj.GetEnumerator(); iter.IsOver(); iter.Next())
        iter.Current();
        
    

*/

#ifndef __LINQ_HPP__
#define __LINQ_HPP__

#include <tuple>
#include <functional>
#include <unordered_set>
#include <memory>

namespace leviathan::linq
{

    constexpr auto begin = [](auto& store) { return std::get<0>(store); };
    constexpr auto end = [](auto& store) { return std::get<1>(store); };
    constexpr auto next = [](auto&& iter) { return std::next(iter); };
    constexpr auto prev = [](auto&& iter) { return std::prev(iter); };
    constexpr auto deref = [](auto&& iter) -> decltype(auto) { return *iter; };
    constexpr auto equal = std::equal_to<void>();

    enum
    {
        ITER_PAIR = 0, BEGIN, END, NEXT, PREV, DEREF, IS_OVER
    };

    template <typename Storage>
    class linq
    {
    private:
        Storage m_store;
        linq(Storage store) : m_store{store}
        {
        }
    public:
        template <typename Range>
        friend from(Range& ranges);

    };

    template <typename Range>
    auto from(Range& ranges)
    {
        auto pair_iter = std::make_pair(std::begin(ranges), std::end(ranges));
        auto tuple = std::make_tuple(pair_iter, begin, end, next, prev, deref, equal);
        return linq<decltype(tuple)>(tuple);
    }

}


#endif