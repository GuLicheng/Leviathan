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

    constexpr auto begin = [](auto& __iter_pair) { return std::get<0>(__iter_pair); };
    constexpr auto end = [](auto& __iter_pair) { return std::get<1>(__iter_pair); };
    constexpr auto next = [](auto&& __iter) { return std::next(__iter); };
    constexpr auto prev = [](auto&& __iter) { return std::prev(__iter); };
    constexpr auto deref = [](auto&& __iter) -> decltype(auto) { return *__iter; };
    constexpr auto equal = std::equal_to<void>();

    template <typename T, typename Storage>
    class linq
    {
    private:
        template <typename _T, typename _Storage>
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

        using value_type = T;

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

            return linq<T, decltype(_store)>{std::move(_store)};          
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
            using TSource = decltype(_deref(m_begin(iter_pair)));
            return linq<TSource, decltype(_store)>{std::move(_store)};

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
            return linq<T, decltype(_store)>{std::move(_store)};
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
            return linq<T, decltype(_store)>{std::move(_store)};
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

            auto r_last_iter = m_prev(m_begin(iter_pair));
            auto _prev = [=](auto&& iter)
            {
                if (i < count)
                {
                    const_cast<int&>(i) ++;
                    return m_prev(iter);
                }
                return r_last_iter;
            };

            auto _store = this->move_as_tuple(iter_pair, m_begin, m_end, _next, _prev, m_deref, m_equal);
            return linq<T, decltype(_store)>{std::move(_store)};
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

            auto r_last_iter = m_prev(m_begin(iter_pair));
            auto _prev = [=](auto&& iter)
            {
                if (m_equal(iter, r_last_iter))
                    return r_last_iter;
                auto prev_iter = m_prev(std::move(iter));
                if (predicate(m_deref(prev_iter)))
                    return prev_iter;
                return last_iter;
            };

            auto _begin = [=](auto& storage)
            {
                auto first = m_begin(storage);
                if (predicate(m_deref(first)))    
                    return first;
                return m_end(storage);
            };

            // auto _end = [=](auto&& storage)
            // {
            //     auto first = m_prev(m_end(storage));
            //     if (predicate(m_deref(first)))
            //         return first;
            //     return m_prev(storage);
            // };

            auto _store = this->move_as_tuple(iter_pair, _begin, m_end, _next, _prev, m_deref, m_equal);
            return linq<T, decltype(_store)>{std::move(_store)};

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
            return linq<T, decltype(_store)>{std::move(_store)};
        }
    
        // concat
        template <typename... Ts>
        constexpr auto concat(linq<Ts...> sequence)
        {
            GetLinqMember();

            auto r_iter_pair = sequence.template get<ITER_PAIR>();
            auto r_begin = sequence.template get<BEGIN>(); 
            auto r_end = sequence.template get<END>(); 
            auto r_next = sequence.template get<NEXT>(); 
            auto r_prev = sequence.template get<PREV>(); 
            auto r_deref = sequence.template get<DEREF>(); 
            auto r_equal = sequence.template get<EQUAL>();

            
            auto first1 = m_begin(iter_pair);
            auto first2 = r_begin(r_iter_pair);
            auto last1 = m_end(iter_pair);
            auto last2 = r_end(r_iter_pair);
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
            auto _store = this->move_as_tuple(
                        std::make_tuple(iter_pair, r_iter_pair, false), 
                        _begin, 
                        _end, 
                        _next, 
                        m_prev, 
                        _deref, 
                        _equal);
            return linq<T, decltype(_store)>{std::move(_store)};
        }
    
        auto distinct() 
        {
            using hash_table_t = std::unordered_set<value_type>;
            hash_table_t table;
            GetLinqMember();
            auto last_iter = m_end(iter_pair);
            auto _next = [=](auto&& iter)
            {
                const_cast<hash_table_t&>(table).emplace(m_deref(iter));
                auto next_iter = m_next(iter);
                while (!m_equal(next_iter, last_iter) && table.count(m_deref(next_iter)))
                {
                    next_iter = m_next(std::move(next_iter));
                }
                return next_iter;
            };
            
            // auto r_last_iter = m_prev(m_begin(iter_pair));
            // auto _prev = [=](auto&& iter)
            // {
            //     const_cast<hash_table_t&>(table).emplace(m_deref(iter));
            //     auto prev_iter = m_prev(iter);
            //     while (!m_equal(prev_iter, r_last_iter) && table.count(m_deref(prev_iter)))
            //     {
            //         prev_iter = m_prev(std::move(prev_iter));
            //     }
            //     return prev_iter;
            // };

            auto _store = this->move_as_tuple(iter_pair, m_begin, m_end, _next, m_prev, m_deref, m_equal);
            return linq<T, decltype(_store)>{std::move(_store)};
        }

        template <typename Selector>
        auto ordered_by(Selector selector) 
        {
            GetLinqMember();
            using TResour = decltype(selector(m_deref(m_begin(iter_pair))));
            static_assert(std::is_same_v<std::decay_t<TResour>, TResour>);
            auto vec_ptr = std::make_shared<std::vector<TResour>>();
            auto first = m_begin(iter_pair);
            auto last = m_end(iter_pair);
            for (auto iter = first; !m_equal(iter, last); iter = m_next(std::move(iter)))
            {
                vec_ptr->emplace_back(m_deref(iter));
            }
            std::sort(vec_ptr->begin(), vec_ptr->end(), [=](const auto& lhs, const auto& rhs)
            {
                return selector(lhs) < selector(rhs);
            });
            // auto _store = std::make_tuple(vec_ptr);
            auto _begin = [=](auto& storage)
            {
                return std::get<0>(storage)->begin();
            };
            auto _end = [=](auto& storage)
            {
                return std::get<0>(storage)->end();
            };

            // reset all operators
            auto _store = this->move_as_tuple(std::make_tuple(vec_ptr), _begin, _end, next, prev, deref, equal);
            return linq<TResour, decltype(_store)>{std::move(_store)};

        }

        int count()
        {
            GetLinqMember();
            int res = 0;
            auto first = m_begin(iter_pair);
            auto last = m_end(iter_pair);
            for (auto iter = first; !m_equal(iter, last); iter = m_next(std::move(iter))) ++res;
            return res;
        }


        struct linq_iterator
        {
            linq* data;

            decltype(auto) operator*() 
            {
                auto&& _store = data->get<ITER_PAIR>();
            }

            using value_type = typename linq::value_type;
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            // using reference_type = decltype()
        };

    };

#undef GetLinqMember



    template <typename BidirectionalRange>
    auto from(BidirectionalRange& __r)
    {
        using value_type = typename std::iterator_traits<decltype(std::begin(__r))>::value_type;
        auto iter_pair = std::make_tuple(std::begin(__r), std::end(__r));
        auto tuple = std::make_tuple(iter_pair, begin, end, next, prev, deref, equal);
        return linq<value_type, decltype(tuple)>(tuple);
    }

} // end of namespace leviathan::linq



#endif