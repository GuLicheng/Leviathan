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
        // our store may contain hashtable which must be empty so don't need to take care of deepcopy

        linq(Storage store) : m_store{std::move(store)}
        {
        }

        // copy
        template <size_t... Idx>
        auto get() const noexcept
        {
            return std::make_tuple(std::get<Idx>(this->m_store)...);
        }

    public:


        template <typename Range>
        friend auto from(Range& __r);

        template <typename Transform>
        constexpr linq& for_each(Transform&& transform)
        {
            auto [store, is_over, next, deref, begin, end] 
                = this->get<ITER_PAIR, EQUAL, NEXT, DEREF, BEGIN, END>();
            const auto first = begin(store);
            const auto last = end(store);
            for (auto iter = first; !is_over(iter, last); iter = next(std::move(iter)))
            {
                transform(deref(iter));
            }
            return *this;
        }

        constexpr auto reverse()
        {
            auto [m_end, m_prev, m_begin, m_next] =
                        this->get<END, PREV, BEGIN, NEXT>();
            auto _begin = [=](auto& storage)
            {
                auto last_iter = m_end(storage);
                auto last = m_prev(std::move(last_iter));
                return last;
            };

            auto _end = [=](auto& storage)
            {
                auto first_iter = m_begin(storage);
                auto first = m_prev(std::move(first_iter));
                return first;
            };

            auto _prev = [=](auto&& iter) { return m_next(std::move(iter)); };
            auto _next = [=](auto&& iter) { return m_prev(std::move(iter)); };

            auto [iter_pair, m_deref, m_equal] = this->get<ITER_PAIR, DEREF, EQUAL>();

            auto _store = std::make_tuple(
                            std::move(iter_pair),
                            _begin, 
                            _end, 
                            _next, 
                            _prev, 
                            std::move(m_deref),
                            std::move(m_equal));

            return linq<decltype(_store)>{std::move(_store)};          
        }

#if 0
        // filter
        template <typename Pred>
        constexpr auto where(Pred predicate) const
        {
            // exchange ++ and begin
            // auto last_iter = m_end(m_store);
            auto iter_pair = this->get<ITER_PAIR>();
            auto m_end = this->get<END>();
            auto last_iter = m_end(iter_pair);
            auto _next = [=](auto&& iter)
            {
                auto next_iter = this->m_next(std::move(iter));
                while (!this->m_equal(next_iter, last_iter) && !predicate(this->m_deref(next_iter)))
                    next_iter = this->m_next(std::move(next_iter));
                return next_iter;
            };
            
            auto _begin = [=](auto& storage)
            {
                auto first_iter = this->m_begin(storage);
                if (!this->m_equal(first_iter, last_iter) && predicate(this->m_deref(first_iter)))
                    return first_iter;
                return _next(std::move(first_iter));
            };

            // 
            auto rlast_iter = m_prev(m_begin(m_store));
            auto _prev = [=](auto&& iter)
            {
                auto prev_iter = this->m_prev(std::move(iter));
                while (!this->m_equal(prev_iter, rlast_iter) && !predicate(this->m_deref(prev_iter)))
                    prev_iter = this->m_prev(std::move(prev_iter));
                return prev_iter;
            };

            return linq<Storage, decltype(_begin), End, decltype(_next), decltype(_prev), Dereference, Equal, TSource>
                {this->m_store, _begin, this->m_end, _next, _prev, this->m_deref, this->m_equal};
            // return linq<Storage, decltype(_begin), End, decltype(_next), Prev, Dereference, Equal>
                // {this->m_store, _begin, this->m_end, _next, this->m_prev, this->m_deref, this->m_equal};
        }
#endif

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