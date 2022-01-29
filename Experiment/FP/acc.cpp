// use std::accumelate implement some algorithms

#include "base.hpp"

template <typename I, typename S, typename Pred>
auto AnyOf(I first, S last, Pred pred)
{
    // init -> Init 
    // x -> std::iterator_traits<T>::value_type 
    auto binary_op = [&](auto init, auto x) {
        return init || pred(x);
    };
    return std::accumulate(first, last, false, binary_op);
}

template <typename I, typename S, typename Pred>
auto AllOf(I first, S last, Pred pred)
{
    auto binary_op = [&](auto init, auto x) {
        return init && pred(x);
    };
    return std::accumulate(first, last, true, binary_op);
}

template <typename I, typename S, typename Pred>
auto FindIf(I first, S last, Pred pred)
{
    auto binary_op = [&](auto init, auto x) {
        return pred(x) ? init : init + 1;
    };
    auto offset = std::accumulate(first, last, 0, binary_op);
    return std::next(first, offset);
}

template <typename I, typename S, typename Comp = std::less<>>
void InsertionSort(I first, S last, Comp comp = {})
{
    // init -> [, ] sorted
    if (first == last)
        return;

    auto binary_op = [first, &comp](auto init, auto x) {
        auto last_iter = first + init - 1;
        if (comp(x, *last_iter))
        {
            auto i = first + init;
            auto pos = std::upper_bound(first, i, x, comp);
            auto tmp = std::move(*i);
            std::move_backward(pos, i, i + 1);
            *pos = std::move(tmp);
        }
        return init + 1;
    };
    std::accumulate(first + 1, last, 1, binary_op);
}


int main(int argc, char const *argv[])
{
    std::vector<int> collection { 1, 2, 3, 4, 5, 4, 3, 2, 1 };
    auto first = collection.begin(), last = collection.end();
    
    auto less_than = [](int x) { 
        return [=](int y) { return y < x; };
    };

    auto less_than_2 = less_than(2);
    auto less_than_10 = less_than(10);

    std::cout.setf(std::ios_base::boolalpha);

    Println(AnyOf(first, last, less_than_2));
    Println(AllOf(first, last, less_than_10));
    Println(*FindIf(first, last, less_than_10));

    InsertionSort(first, last);

    std::ranges::copy(collection, std::ostream_iterator<int>{std::cout, ", "});
    std::endl(std::cout);

    std::cout.unsetf(std::ios_base::boolalpha);
    return 0;
}


