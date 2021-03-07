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


#define GetLinqMember() \
        auto iter_pair = this->get<ITER_PAIR>(); \
        auto m_begin = this->get<BEGIN>(); \
        auto m_end = this->get<END>(); \
        auto m_next = this->get<NEXT>(); \
        auto m_prev = this->get<PREV>(); \
        auto m_deref = this->get<DEREF>(); \
        auto m_equal = this->get<EQUAL>()

namespace leviathan::linq
{

    enum
    {
        ITER_PAIR = 0, BEGIN, END, NEXT, PREV, DEREF, EQUAL
    };

    template <typename Storage>
    class linq
    {
    private:
        template <typename _Storage>
        friend class linq;

        Storage m_store; 
        // our store may contain hashtable which must be empty so don't need to take care of deepcopy

        linq(Storage store) : m_store{std::move(store)}
        {
        }

        // copy
        template <size_t Idx>
        auto get() noexcept
        {
            return std::get<Idx>(this->m_store);
        }

        template <typename... Ts>
        auto move_as_tuple(Ts&&... ts)
        {
            return std::make_tuple(std::move(ts)...);
        }

    public:


        template <typename Range>
        friend auto from(Range& __r);

        template <typename Transform>
        constexpr linq& for_each(Transform&& transform)
        {
            GetLinqMember();
            const auto first = m_begin(iter_pair);
            const auto last = m_end(iter_pair);
            for (auto iter = first; !m_equal(iter, last); iter = m_next(std::move(iter)))
            {
                transform(m_deref(iter));
            }
            return *this;
        }

        // reverse
        constexpr auto reverse()
        {
            GetLinqMember();
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

            auto _store = this->move_as_tuple(iter_pair, _begin, _end, _next, _prev, m_deref, m_equal);

            return linq<decltype(_store)>{std::move(_store)};          
        }

        // transform
        template <typename Selector>
        constexpr auto select(Selector selector)
        {
            GetLinqMember();
            auto _deref = [=](auto&& iter) -> decltype(auto)
            {
                auto&& val = m_deref(iter);
                return selector(std::forward<decltype(val)>(val));
            };

            auto _store = this->move_as_tuple(iter_pair, m_begin, m_end, m_next, m_prev, _deref, m_equal);
            return linq<decltype(_store)>{std::move(_store)};

        }

        // drop
        constexpr auto skip(int count) 
        {
            GetLinqMember();
            auto _begin = [=] (auto& storage)
            {
                auto first_iter = m_begin(storage);
                auto last_iter = m_end(storage);
                auto _count = count;
                while (!m_equal(first_iter, last_iter) && _count--)
                {
                    first_iter = m_next(std::move(first_iter));
                }
                return first_iter;
            };

            auto _store = this->move_as_tuple(iter_pair, _begin, m_end, m_next, m_prev, m_deref, m_equal);
            return linq<decltype(_store)>{std::move(_store)};
        }
    
        // drop_while
        template <typename Pred>
        constexpr auto skip_while(Pred predicate) 
        {
            GetLinqMember();
            auto _begin = [=] (auto& storage)
            {
                auto first_iter = m_begin(storage);
                auto last_iter = m_end(storage);
                while (!m_equal(first_iter, last_iter) && predicate(m_deref(first_iter)))
                {
                    first_iter = m_next(std::move(first_iter));
                }
                return first_iter;
            };

            auto _store = this->move_as_tuple(iter_pair, _begin, m_end, m_next, m_prev, m_deref, m_equal);
            return linq<decltype(_store)>{std::move(_store)};
        }
    

        constexpr auto take(int count)
        {
            GetLinqMember();
            auto last_iter = m_end(iter_pair);
            int i = 1;
            auto _next = [=](auto&& iter) 
            {
                if (i < count)
                {
                    // ++i;
                    const_cast<int&>(i) ++;
                    return m_next(iter);
                }
                return last_iter;
            };

            auto _store = this->move_as_tuple(iter_pair, m_begin, m_end, _next, m_prev, m_deref, m_equal);
            return linq<decltype(_store)>{std::move(_store)};
        }

        // take while
        template <typename Pred>
        constexpr auto take_while(Pred predicate) 
        {
            GetLinqMember();

            auto last_iter = m_end(iter_pair);
            auto _next = [=](auto&& iter)
            {
                if (m_equal(iter, last_iter))
                    return last_iter;
                auto next_iter = m_next(std::move(iter));
                if (predicate(m_deref(next_iter)))
                    return next_iter;
                return last_iter;
            };

            auto _begin = [=](auto& storage)
            {
                auto first = m_begin(storage);
                if (predicate(m_deref(first)))    
                    return first;
                return m_end(storage);
            };

            auto _store = this->move_as_tuple(iter_pair, _begin, m_end, _next, m_prev, m_deref, m_equal);
            return linq<decltype(_store)>{std::move(_store)};

        }        
    
        // filter
        template <typename Pred>
        constexpr auto where(Pred predicate)
        {
            GetLinqMember();
            // exchange ++ and begin
            auto last_iter = m_end(iter_pair);
            auto _next = [=](auto&& iter)
            {
                auto next_iter = m_next(std::move(iter));
                while (!m_equal(next_iter, last_iter) && !predicate(m_deref(next_iter)))
                    next_iter = m_next(std::move(next_iter));
                return next_iter;
            };
            
            auto _begin = [=](auto& storage)
            {
                auto first_iter = m_begin(storage);
                if (!m_equal(first_iter, last_iter) && predicate(m_deref(first_iter)))
                    return first_iter;
                return _next(std::move(first_iter));
            };

            // 
            auto rlast_iter = m_prev(m_begin(iter_pair));
            auto _prev = [=](auto&& iter)
            {
                auto prev_iter = m_prev(std::move(iter));
                while (!m_equal(prev_iter, rlast_iter) && !predicate(m_deref(prev_iter)))
                    prev_iter = m_prev(std::move(prev_iter));
                return prev_iter;
            };

            auto _store = this->move_as_tuple(iter_pair, _begin, m_end, _next, _prev, m_deref, m_equal);
            return linq<decltype(_store)>{std::move(_store)};
        }
    
        // concat
        template <typename T>
        constexpr auto concat(linq<T> sequence)
        {
            GetLinqMember();
            // auto r_iter_pair = sequence.get<ITER_PAIR>();
            auto r_begin = sequence.template get<BEGIN>(); 
            auto r_end = sequence.template get<END>(); 
            auto r_next = sequence.template get<NEXT>(); 
            auto r_prev = sequence.template get<PREV>(); 
            auto r_deref = sequence.template get<DEREF>(); 
            auto r_equal = sequence.template get<EQUAL>();

            auto _store1 = iter_pair;  // iter_pair
            auto _store2 = sequence.template get<ITER_PAIR>();
            auto _store = std::make_tuple(_store1, _store2, false);
            auto first1 = m_begin(_store1);
            auto first2 = r_begin(_store2);
            auto last1 = m_end(_store1);
            auto last2 = r_end(_store2);
            // if iter ref second sequence, _store<3> will be true
            auto _begin = [=](auto& storage)
            {
                return std::make_tuple(std::move(first1), std::move(first2), false);
            };

            auto _end = [=](auto& storage)
            {
                return std::make_tuple(std::move(last1), std::move(last2), true);
            };

            auto _next = [=](auto&& iter_pair)
            {
                // auto [f, s, is_second] = iter_pair;
                auto [f, s, is_second] = iter_pair;
                if (!is_second && !m_equal(f, last1))
                {
                    // first iterator
                    f = m_next(std::move(f));
                    if (m_equal(f, last1))
                        return std::make_tuple(std::move(f), std::move(s), true);
                    return std::make_tuple(std::move(f), std::move(s), is_second);
                }
                else if (!is_second && m_equal(f, last1))
                {   
                    return std::make_tuple(std::move(f), std::move(s), false);
                }
                else
                {
                    return std::make_tuple(std::move(f), r_next(s), is_second);
                }
            };


            auto _deref = [=](auto&& iter_pair) -> decltype(auto)
            {
                if (m_equal(std::get<0>(iter_pair), last1))
                {
                    return r_deref(std::get<1>(iter_pair));
                }
                else
                {
                    return m_deref(std::get<0>(iter_pair));
                }
            };

            auto _equal = [=](const auto& __lhs, const auto& __rhs)
            {
                return m_equal(std::get<0>(__lhs), std::get<0>(__rhs)) &&
                        r_equal(std::get<1>(__lhs), std::get<1>(__rhs));
            };

            // _prev should be changed also, but I just simplifer it
            auto __store = this->move_as_tuple(_store, _begin, _end, _next, m_prev, _deref, _equal);
            return linq<decltype(__store)>{std::move(__store)};
            // return linq<decltype(_store), decltype(_begin), decltype(_end), decltype(_next), Prev, decltype(_deref), decltype(_equal), TSource>
                // {_store, _begin, _end, _next, this->m_prev, _deref, _equal};
        }
    
    };

#undef GetLinqMember

    constexpr auto begin = [](auto& __iter_pair) { return std::get<0>(__iter_pair); };
    constexpr auto end = [](auto& __iter_pair) { return std::get<1>(__iter_pair); };
    constexpr auto next = [](auto&& __iter) { return std::next(__iter); };
    constexpr auto prev = [](auto&& __iter) { return std::prev(__iter); };
    constexpr auto deref = [](auto&& __iter) -> decltype(auto) { return *__iter; };
    constexpr auto equal = std::equal_to<void>();

    template <typename BidirectionalRange>
    auto from(BidirectionalRange& __r)
    {
        auto iter_pair = std::make_tuple(std::begin(__r), std::end(__r));
        auto tuple = std::make_tuple(iter_pair, begin, end, next, prev, deref, equal);
        return linq<decltype(tuple)>(tuple);
    }

} // end of namespace leviathan::linq



#endif