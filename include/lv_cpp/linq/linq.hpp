/*
    Just a tool
*/

#ifndef __LINQ_HPP__
#define __LINQ_HPP__

#include <functional>
#include <tuple>
#include <iostream>
#include <iterator>
#include <limits>

#include <lv_cpp/io/console.hpp>

namespace leviathan::linq
{

    struct invalid_pair_iterator : std::exception
    {
        constexpr const char* what() const noexcept override
        {
            return "Linq out of range";
        }
    };

    // for iterator, we must support *, ++, ==, !=
    
    auto default_dereference = [](auto iter) -> auto& { return *iter; };
    auto identity = [](auto iter) { return iter; };
    auto next = [](auto iter) { return std::next(iter); };
    auto equal = std::equal_to<>();

    template <typename Storage, typename First,
              typename Last, typename Next, typename Deref>
    class linq
    {
        [[no_unique_address]]Next nextFunc; // forward iterator a step
        [[no_unique_address]]First firstFunc; // return first element
        [[no_unique_address]]Deref derefFunc; // return *iter
        [[no_unique_address]]Last lastFunc; // return last or default
        Storange iter_pair; // store begin and end iterator
        using value_type = typename std::iterator_traits<Iter>::value_type;
    public:

        linq(Iter first, Sent last, First firstFunc, Last lastFunc, Next next, Deref deref)
            : iterator{first}, sentinel{last}, firstFunc{firstFunc}, lastFunc{lastFunc},
              nextFunc{next}, derefFunc{deref}
        {
        }

        template <typename Pred>
        auto where(Pred predicate) const
        {
            auto new_next = [=](Iter iter) 
            {
                auto forward = this->nextFunc(iter);
                if (forward == this->sentinel || predicate(*forward))
                {
                    // console::write_line(*f)
                    return forward;
                }
                else
                {
                    while (forward != this->sentinel && !predicate(*forward))
                        forward = this->nextFunc(forward);
                }
                return forward;
            };

            using function_type = decltype(new_next);
            return linq<Iter, Sent, function_type, Last, function_type, Deref>
                {this->iterator, this->sentinel, new_next, this->lastFunc, new_next, this->derefFunc};
        }


        template <typename Apply>
        const linq& for_each(Apply apply) const
        {
            for (auto iter = firstFunc(iterator); iter != lastFunc(sentinel); iter = nextFunc(iter))
                apply(derefFunc(iter));
            return *this;
        }

        template <typename BinaryOp, typename TResult = value_type>
        TResult reduce(BinaryOp op, TResult init = TResult{}) const
        {
            for (auto iter = firstFunc(iterator); iter != lastFunc(sentinel); iter = nextFunc(iter))
            {
                // std::cout << init << std::endl;
                init = op(derefFunc(iter), std::move(init));
            }
            return init;
        }
        
        template <typename TResult = value_type>
        TResult sum(TResult init = {}) const
        {
            return reduce(std::plus<>(), std::move(init));
        }
/*
        template <typename Container>
        Container to() const
        {
            return Container(iterator, sentinel);
        }

        template <template <typename...> typename Container>
        Container<value_type> to() const
        {
            return Container<value_type>(iterator, sentinel);
        }
*/


    }; // class linq

    template <typename Container>
    auto from(Container& c)
    {
        // template <typename Iter, typename Sent, typename First, typename Last, 
        // typename Next, typename Deref>
        return linq{std::begin(c), std::end(c), identity, identity, next, default_dereference};
    }

} // namespace linq


#endif
