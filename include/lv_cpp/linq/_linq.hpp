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

        template <size_t N>
        constexpr auto get() const noexcept
        {
            return std::get<N>(this->m_store);
        }
        
    public:


        template <typename Range>
        friend auto from(Range& __r);

        template <typename Transform>
        constexpr linq for_each(Transform&& transform) const
        {
            auto store = this->get<ITER_PAIR>();
            auto first = this->get<BEGIN>()(store);
            auto last = this->get<END>()(store);
            auto is_over = this->get<EQUAL>();
            auto next = this->get<NEXT>();
            auto deref = this->get<DEREF>();
            for (auto iter = first; !is_over(iter, last); iter = next(std::move(iter)))
            {
                transform(deref(iter));
            }
            return *this;
        }

        constexpr auto reverse() const
        {
            auto m_end = this->get<END>();
            auto m_prev = this->get<PREV>();
            auto m_begin = this->get<BEGIN>();
            auto _begin = [=](auto& storage)
            {
                auto last_iter = this->m_end(storage);
                auto last = this->m_prev(std::move(last_iter));
                return last;
            };

            auto _end = [=](auto& storage)
            {
                auto first_iter = this->m_begin(storage);
                auto first = this->m_prev(std::move(first_iter));
                return first;
            };

            auto _prev = [=](auto&& iter) { return m_next(std::move(iter)); };
            auto _next = [=](auto&& iter) { return m_prev(std::move(iter)); };

            auto _store = std::make_tuple(this->get<ITER_PAIR>(), _begin, _end, _next, _prev, 
                                    this->get<DEREF>(), this->get<EQUAL>());

            return linq<decltype(_store)>{std::move(_store)};          
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