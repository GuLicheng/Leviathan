/*
    Just a tool
*/

#ifndef __LINQ_HPP__
#define __LINQ_HPP__

#include <iostream>
// #include <exception>
#include <iterator>
#include <tuple>
#include <unordered_set>
#include <functional>

namespace leviathan::linq
{
    
    // struct invalid_pair_iterator : std::exception
    // {
    //     constexpr const char* what() const noexcept override
    //     {
    //         return "Linq out of range";
    //     }
    // };

    // for iterator, it may overloaded some operators such as
    // ++, --, *, ==, != ...
    template <typename Storage, typename Begin, typename End, typename Next, typename Prev, typename Dereference, typename Equal>       
    class linq
    {
    public:
        [[no_unique_address]]Begin m_begin;         
        [[no_unique_address]]End m_end;             
        [[no_unique_address]]Next m_next;           
        [[no_unique_address]]Prev m_prev;         
        [[no_unique_address]]Dereference m_deref;   
        [[no_unique_address]]Equal m_equal;         

        Storage m_store;                           // store iterator
        using self = linq;
    public:
        // using value_type = TSource;

        using value_type = decltype(m_deref(m_begin(m_store)));

        linq(Storage storage, Begin begin, End end, Next next, Prev prev, Dereference deref, Equal equal)
            : m_store{storage}, m_begin{begin}, m_end{end}, m_next{next}, m_prev{prev}, m_deref{deref}, m_equal{equal}
        {
        }

        template <typename Transform>
        linq for_each(Transform transform) const
        {
            auto first = m_begin(m_store);
            auto last = m_end(m_store);
            for (auto iter = first; !m_equal(iter, last); iter = m_next(iter))
            {
                transform(m_deref(iter));
            }
            return *this;
        }

        auto reverse() const
        {
            auto _begin = [=](auto storage)
            {
                auto last_iter = this->m_end(storage);
                auto last = this->m_prev(last_iter);
                return last;
            };
            auto _end = [=](auto storage)
            {
                auto first_iter = this->m_begin(storage);
                auto first = this->m_prev(first_iter);
                return first;
            };

            auto _prev = [=](auto iter) { return this->m_next(iter); };
            auto _next = [=](auto iter) { return this->m_prev(iter); };

            return linq<Storage, decltype(_begin), decltype(_end), decltype(_next), decltype(_prev), Dereference, Equal>
                {this->m_store, _begin, _end, _next, _prev, this->m_deref, this->m_equal};            
        }     
        
        // filter
        template <typename Pred>
        auto where(Pred predicate) const
        {
            // exchange ++ and begin
            auto store = this->m_store;
            auto _next = [=](auto iter)
            {
                auto next_iter = this->m_next(iter);
                auto end = this->m_end(store);
                while (!this->m_equal(next_iter, end) && !predicate(this->m_deref(next_iter)))
                    next_iter = this->m_next(next_iter);
                return next_iter;
            };
            
            auto _begin = [=](auto storage)
            {
                auto first_iter = this->m_begin(storage);
                auto end = this->m_end(store);
                if (!this->m_equal(first_iter, end) && predicate(this->m_deref(first_iter)))
                    return first_iter;
                return _next(first_iter);
            };

            return linq<Storage, decltype(_begin), End, decltype(_next), Prev, Dereference, Equal>
                {this->m_store, _begin, this->m_end, _next, this->m_prev, this->m_deref, this->m_equal};
        }

        // transform
        template <typename Selector>
        auto select(Selector selector) const
        {
            auto _deref = [=](auto iter) -> decltype(auto)
            {
                auto&& val = this->m_deref(iter);
                return selector(std::forward<decltype(val)>(val));
            };
            return linq<Storage, Begin, End, Next, Prev, decltype(_deref), Equal>
                {this->m_store, this->m_begin, this->m_end, this->m_next, this->m_prev, _deref, this->m_equal};
        }

        // drop
        auto skip(int count) const
        {
            auto _begin = [=] (auto storage)
            {
                auto first_iter = this->m_begin(storage);
                auto last_iter = this->m_end(storage);
                auto _count = count;
                while (!this->m_equal(first_iter, last_iter) && _count--)
                {
                    first_iter = this->m_next(first_iter);
                }
                return first_iter;
            };
            return linq<Storage, decltype(_begin), End, Next, Prev, Dereference, Equal>
                {this->m_store, _begin, this->m_end, this->m_next, this->m_prev, this->m_deref, this->m_equal};
        }

        // drop_while
        template <typename Pred>
        auto skip_while(Pred predicate) const
        {
            auto _begin = [=] (auto storage)
            {
                auto first_iter = this->m_begin(storage);
                auto last_iter = this->m_end(storage);
                while (!this->m_equal(first_iter, last_iter) && predicate(this->m_deref(first_iter)))
                {
                    first_iter = this->m_next(first_iter);
                }
                return first_iter;
            };
            return linq<Storage, decltype(_begin), End, Next, Prev, Dereference, Equal>
                {this->m_store, _begin, this->m_end, this->m_next, this->m_prev, this->m_deref, this->m_equal};
        }

        auto take(int count) const
        {
            auto last_iter = m_end(m_store);
            int i = 1;
            auto _next = [=](auto iter) 
            {
                if (i < count)
                {
                    // ++i;
                    const_cast<int&>(i) ++;
                    return this->m_next(iter);
                }
                return last_iter;
            };

            return linq<Storage, Begin, End, decltype(_next), Prev, Dereference, Equal>
                {this->m_store, this->m_begin, this->m_end, _next, this->m_prev, this->m_deref, this->m_equal};
        }

        template <typename Pred>
        auto take_while(Pred predicate) const
        {
            auto last_iter = m_end(m_store);
            auto _next = [=](auto iter)
            {
                if (this->m_equal(iter, last_iter))
                    return last_iter;
                if (predicate(this->m_deref(iter)))
                    return this->m_next(iter);
                return last_iter;
            };
            return linq<Storage, Begin, End, decltype(_next), Prev, Dereference, Equal>
                {this->m_store, this->m_begin, this->m_end, _next, this->m_prev, this->m_deref, this->m_equal};

        }

        auto distinct() const
        {
            using hash_table_t = std::unordered_set<value_type>;
            hash_table_t table;
            auto last_iter = m_end(m_store);
            auto _next = [=](auto iter)
            {
                const_cast<hash_table_t&>(table).emplace(this->m_deref(iter));
                auto next_iter = this->m_next(iter);
                while (!this->m_equal(next_iter, last_iter) && table.count(this->m_deref(next_iter)))
                {
                    next_iter = this->m_next(next_iter);
                }
                return next_iter;
            };
            return linq<Storage, Begin, End, decltype(_next), Prev, Dereference, Equal>
                {this->m_store, this->m_begin, this->m_end, _next, this->m_prev, this->m_deref, this->m_equal};
        }

#if 0
        // concat
        template <typename... Ts>
        auto concat(linq<Ts...>& sequence)
        {
            return 0;
        }

        // repeat
        auto repeat(int count) const 
        {
            auto first_iter = m_begin(m_store);
            auto last_iter = m_end(m_store);

        }

        // zip
        template <typename... Ts>
        auto zip(const linq<TSource, Ts...>& source) const;

        template <typename... Ts>
        auto concat(const linq<TSource, Ts...> source) const;


#endif
    };
    
    
    // Utils
     
    inline constexpr auto begin = [](auto store) { return std::get<0>(store); };
    inline constexpr auto end = [](auto store) { return std::get<1>(store); };
    inline constexpr auto next = [](auto iter) { return std::next(iter); };
    inline constexpr auto prev = [](auto iter) { return std::prev(iter); };
    inline constexpr auto deref = [](auto iter) -> auto& { return *iter; };
    inline constexpr auto equal = std::equal_to<void>();


    template <typename Container>
    auto from(const Container& container)
    {
        using value_type = typename std::iterator_traits<decltype(std::begin(container))>::value_type;
        using storage_type = std::tuple<decltype(std::begin(container)), decltype(std::end(container))>;
        return linq<storage_type, decltype(begin), decltype(end), decltype(next), decltype(prev), decltype(deref), decltype(equal)>{
            std::make_tuple(std::begin(container), std::end(container)), 
            begin, end, next, prev, deref, equal};
    }


} // namespace leviathan:linq



#endif
