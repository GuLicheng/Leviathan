/*
    Just a tool, 
        cpp standard: 
            lambda closure types are non-literal types before C++17 -> c++17 
            decltype(auto) -> c++14
            rest -> c++11
        
*/


#include "_linq.hpp"

#ifndef __LINQ_HPP__
#define __LINQ_HPP__



#include <iostream>
// #include <exception>
#include <iterator>
#include <tuple>
#include <unordered_set>
#include <functional>
#include <memory>

namespace leviathan::linq
{

    // Utils, prev shoule not be such simply
     
    inline constexpr auto begin = [](auto& store) { return std::get<0>(store); };
    inline constexpr auto end = [](auto& store) { return std::get<1>(store); };
    inline constexpr auto next = [](auto&& iter) { return std::next(iter); };
    inline constexpr auto prev = [](auto&& iter) { return std::prev(iter); };
    inline constexpr auto deref = [](auto&& iter) -> decltype(auto) { return *iter; };
    inline constexpr auto equal = std::equal_to<void>();

    // struct invalid_pair_iterator : std::exception
    // {
    //     constexpr const char* what() const noexcept override
    //     {
    //         return "Linq out of range";
    //     }
    // };

    // for iterator, it may overloaded some operators such as
    // ++, --, *, ==, != ...
    // TSource now is useless
    template <typename Storage, typename Begin, typename End, typename Next, typename Prev, typename Dereference, typename Equal, typename TSource>       
    class linq
    {
    // public:
        [[no_unique_address]]Begin m_begin;         
        [[no_unique_address]]End m_end;             
        [[no_unique_address]]Next m_next;           
        [[no_unique_address]]Prev m_prev;         
        [[no_unique_address]]Dereference m_deref;   
        [[no_unique_address]]Equal m_equal;         

        Storage m_store;                           // store iterator
        using self = linq;

        template <typename _Storage, typename _Begin, 
                typename _End, typename _Next, typename _Prev, 
                typename _Dereference, typename _Equal, typename _TSource>       
        friend class linq;

public:

    public:
        using value_type = TSource;

        // using value_type = std::decay_t<decltype(m_deref(m_begin(m_store)))>;

        constexpr linq(Storage storage, Begin begin, End end, Next next, Prev prev, Dereference deref, Equal equal)
            : m_store{storage}, m_begin{begin}, m_end{end}, m_next{next}, m_prev{prev}, m_deref{deref}, m_equal{equal}
        {
        }

        template <typename Transform>
        constexpr linq for_each(Transform&& transform) const
        {
            auto first = m_begin(m_store);
            auto last = m_end(m_store);
            for (auto iter = first; !m_equal(iter, last); iter = m_next(std::move(iter)))
            {
                transform(m_deref(iter));
            }
            return *this;
        }

        constexpr auto reverse() const
        {
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

            auto _prev = [=](auto&& iter) { return this->m_next(std::move(iter)); };
            auto _next = [=](auto&& iter) { return this->m_prev(std::move(iter)); };

            return linq<Storage, decltype(_begin), decltype(_end), decltype(_next), decltype(_prev), Dereference, Equal, TSource>
                {this->m_store, _begin, _end, _next, _prev, this->m_deref, this->m_equal};            
        }     
        
        // filter
        template <typename Pred>
        constexpr auto where(Pred predicate) const
        {
            // exchange ++ and begin
            auto last_iter = m_end(m_store);
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

        // transform
        template <typename Selector>
        constexpr auto select(Selector selector) const
        {
            auto _deref = [=](auto&& iter) -> decltype(auto)
            {
                auto&& val = this->m_deref(iter);
                return selector(std::forward<decltype(val)>(val));
            };
            using TResult = std::decay_t<decltype(_deref(this->m_begin(this->m_store)))>;
            return linq<Storage, Begin, End, Next, Prev, decltype(_deref), Equal, TResult>
                {this->m_store, this->m_begin, this->m_end, this->m_next, this->m_prev, _deref, this->m_equal};
        }

        // drop
        constexpr auto skip(int count) const
        {
            auto _begin = [=] (auto& storage)
            {
                auto first_iter = this->m_begin(storage);
                auto last_iter = this->m_end(storage);
                auto _count = count;
                while (!this->m_equal(first_iter, last_iter) && _count--)
                {
                    first_iter = this->m_next(std::move(first_iter));
                }
                return first_iter;
            };
            return linq<Storage, decltype(_begin), End, Next, Prev, Dereference, Equal, TSource>
                {this->m_store, _begin, this->m_end, this->m_next, this->m_prev, this->m_deref, this->m_equal};
        }

        // drop_while
        template <typename Pred>
        constexpr auto skip_while(Pred predicate) const
        {
            auto _begin = [=] (auto storage)
            {
                auto first_iter = this->m_begin(storage);
                auto last_iter = this->m_end(storage);
                while (!this->m_equal(first_iter, last_iter) && predicate(this->m_deref(first_iter)))
                {
                    first_iter = this->m_next(std::move(first_iter));
                }
                return first_iter;
            };
            return linq<Storage, decltype(_begin), End, Next, Prev, Dereference, Equal, TSource>
                {this->m_store, _begin, this->m_end, this->m_next, this->m_prev, this->m_deref, this->m_equal};
        }

        constexpr auto take(int count) const
        {
            auto last_iter = m_end(m_store);
            int i = 1;
            auto _next = [=](auto&& iter) 
            {
                if (i < count)
                {
                    // ++i;
                    const_cast<int&>(i) ++;
                    return this->m_next(iter);
                }
                return last_iter;
            };

            return linq<Storage, Begin, End, decltype(_next), Prev, Dereference, Equal, TSource>
                {this->m_store, this->m_begin, this->m_end, _next, this->m_prev, this->m_deref, this->m_equal};
        }

        template <typename Pred>
        constexpr auto take_while(Pred predicate) const
        {
            auto last_iter = m_end(m_store);
            auto _next = [=](auto&& iter)
            {
                if (this->m_equal(iter, last_iter))
                    return last_iter;
                auto next_iter = this->m_next(std::move(iter));
                if (predicate(this->m_deref(next_iter)))
                    return next_iter;
                return last_iter;
            };

            auto _begin = [=](auto& storage)
            {
                auto first = this->m_begin(storage);
                if (predicate(this->m_deref(first)))    
                    return first;
                return this->m_end(storage);
            };

            return linq<Storage, decltype(_begin), End, decltype(_next), Prev, Dereference, Equal, TSource>
                {this->m_store, _begin, this->m_end, _next, this->m_prev, this->m_deref, this->m_equal};

        }

        auto distinct() const
        {
            using hash_table_t = std::unordered_set<TSource>;
            hash_table_t table;
            auto last_iter = m_end(m_store);
            auto _next = [=](auto&& iter)
            {
                const_cast<hash_table_t&>(table).emplace(this->m_deref(iter));
                auto next_iter = this->m_next(iter);
                while (!this->m_equal(next_iter, last_iter) && table.count(this->m_deref(next_iter)))
                {
                    next_iter = this->m_next(std::move(next_iter));
                }
                return next_iter;
            };
            return linq<Storage, Begin, End, decltype(_next), Prev, Dereference, Equal, TSource>
                {this->m_store, this->m_begin, this->m_end, _next, this->m_prev, this->m_deref, this->m_equal};
        }

        // concat
        template <typename... Ts>
        constexpr auto concat(const linq<Ts...>& sequence) const
        {
            auto _store1 = this->m_store;  // iter_pair
            auto _store2 = sequence.m_store;
            auto rhs = &sequence;
            auto _store = std::make_tuple(_store1, _store2, false);
            auto first1 = this->m_begin(_store1);
            auto first2 = rhs->m_begin(_store2);
            auto last1 = this->m_end(_store1);
            auto last2 = rhs->m_end(_store2);
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
                if (!is_second && !this->m_equal(f, last1))
                {
                    // first iterator
                    f = this->m_next(std::move(f));
                    if (this->m_equal(f, last1))
                        return std::make_tuple(std::move(f), std::move(s), true);
                    return std::make_tuple(std::move(f), std::move(s), is_second);
                }
                else if (!is_second && this->m_equal(f, last1))
                {   
                    return std::make_tuple(std::move(f), std::move(s), false);
                }
                else
                {
                    return std::make_tuple(std::move(f), rhs->m_next(s), is_second);
                }
            };

            auto _deref = [=](auto&& iter_pair) -> decltype(auto)
            {
                if (this->m_equal(std::get<0>(iter_pair), last1))
                {
                    return rhs->m_deref(std::get<1>(iter_pair));
                }
                else
                {
                    return this->m_deref(std::get<0>(iter_pair));
                }
            };

            auto _equal = [=](const auto& __lhs, const auto& __rhs)
            {
                return this->m_equal(std::get<0>(__lhs), std::get<0>(__rhs)) &&
                        rhs->m_equal(std::get<1>(__lhs), std::get<1>(__rhs));
            };

            // _prev should be changed also, but I just simplifer it
            return linq<decltype(_store), decltype(_begin), decltype(_end), decltype(_next), Prev, decltype(_deref), decltype(_equal), TSource>
                {_store, _begin, _end, _next, this->m_prev, _deref, _equal};
        }

        template <typename Selector>
        auto ordered_by(Selector selector) const
        {
            using TResult = std::decay_t<decltype(selector(this->m_deref(this->m_begin(this->m_store))))>;
            
            auto vec_ptr = std::make_shared<std::vector<TResult>>();
            auto first = m_begin(m_store);
            auto last = m_end(m_store);
            for (auto iter = first; !m_equal(iter, last); iter = m_next(std::move(iter)))
            {
                vec_ptr->emplace_back(m_deref(iter));
            }
            std::sort(vec_ptr->begin(), vec_ptr->end(), [&](const auto& lhs, const auto& rhs)
            {
                return selector(lhs) < selector(rhs);
            });
            auto _store = std::make_tuple(vec_ptr);
            auto _begin = [=](auto& storage)
            {
                return std::get<0>(storage)->begin();
            };
            auto _end = [=](auto& storage)
            {
                return std::get<0>(storage)->end();
            };

            return linq<decltype(_store), decltype(_begin), decltype(_end), decltype(next), decltype(prev), decltype(deref), decltype(equal), TResult>
                { _store, _begin, _end, next, prev, deref, equal};
        }

        int count() const 
        {
            int res = 0;
            auto first = m_begin(m_store);
            auto last = m_end(m_store);
            for (auto iter = first; !m_equal(iter, last); iter = m_next(std::move(iter))) ++res;
            return res;
        }

    };
    
    
    template <typename Container>
    auto from(Container& container)
    {
        using value_type = typename std::iterator_traits<decltype(std::begin(container))>::value_type;
        using storage_type = std::tuple<decltype(std::begin(container)), decltype(std::end(container))>;
        return linq<storage_type, decltype(begin), decltype(end), decltype(next), decltype(prev), decltype(deref), decltype(equal), value_type>{
            std::make_tuple(std::begin(container), std::end(container)), 
            begin, end, next, prev, deref, equal};
    }


} // namespace leviathan:linq



#endif
