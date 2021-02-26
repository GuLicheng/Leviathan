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

    constexpr auto begin = [](auto& __iter_pair) { return std::get<0>(__iter_pair); };
    constexpr auto end = [](auto& __iter_pair) { return std::get<1>(__iter_pair); };
    constexpr auto next = [](auto&& __iter) { return std::next(__iter); };
    constexpr auto prev = [](auto&& __iter) { return std::prev(__iter); };
    constexpr auto deref = [](auto&& __iter) -> decltype(auto) { return *__iter; };
    constexpr auto equal = std::equal_to<void>();

    enum
    {
        ITER_PAIR = 0, BEGIN, END, NEXT, PREV, DEREF, EQUAL
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

        template <typename _Storage>
        friend class linq<_Storage>;

        template <typename Range>
        friend from(Range&);

    };

    template <typename Range>
    auto from(Range& __r)
    {
        auto iter_pair = std::make_tuple(std::begin(__r), std::end(__r));
        auto tuple = std::make_tuple(iter_pair, begin, end, next, prev, deref, equal);
        return linq<decltype(tuple)>(tuple);
    }

}


#endif