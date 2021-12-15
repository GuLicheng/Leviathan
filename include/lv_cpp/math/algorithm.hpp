#ifndef __ALGORITHM_HPP__
#define __ALGORITHM_HPP__

#include <algorithm>
#include <vector>
#include <concepts>

namespace leviathan
{

    // Proj and pass by value, std::move will make algorithm very slowly

    struct insertion_sort_fn
    {
        // for simplifier, use random_access_iterator 
        template <std::random_access_iterator I, std::sentinel_for<I> S, 
                    typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
        constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const
        {
            if (first == last)
                return first;

            auto i = first + 1;
            for (; i != last; ++i)
            {
                auto j = i - 1;
                // if arr[j] <= arr[i] continue
                if (!std::invoke(comp, std::invoke(proj, *i), std::invoke(proj, *j))) continue;
                else
                {
                    auto pos = std::ranges::upper_bound(first, i, *i, std::ref(comp), std::ref(proj));
                    auto tmp = std::move(*i);
                    std::ranges::move(pos, i, pos + 1);
                    *pos = std::move(tmp);
                } 
            }
            return i;
        }

        template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj>
        constexpr std::ranges::borrowed_iterator_t<Range>
        operator()(Range&& r, Comp comp = {}, Proj proj = {}) const
        {
            return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
        }
    };

    inline constexpr insertion_sort_fn insertion_sort{ };


    struct merge_sort_fn
    {
        template <std::random_access_iterator I, std::sentinel_for<I> S, 
            typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
        constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const
        {
            if (last - first > 1)
            {
                auto middle = first + (last - first) / 2;
                (*this)(first, middle, std::ref(comp), std::ref(proj));
                (*this)(middle, last, std::ref(comp), std::ref(proj));
                std::ranges::inplace_merge(first, middle, last, std::ref(comp), std::ref(proj));
            }
            return first;
        }

        template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj>
        constexpr std::ranges::borrowed_iterator_t<Range> operator()(Range&& r, Comp comp = {}, Proj proj = {}) const
        {
            return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
        }
    };

    inline constexpr merge_sort_fn merge_sort{ };


    struct tim_sort_fn
    {
    private:

        constexpr static int MinSize = 32;

        template <typename I, typename S, typename Comp, typename Proj>
        constexpr static I count_run_and_make_ascending(I first, S last, Comp comp, Proj proj) 
        {
            // comp is less relationship
            if (first == last)
                return first; // something may not happened

            auto left = first;
            auto right = left + 1;

            if (right == last)
                return right;

            if (std::invoke(comp, std::invoke(proj, *right), std::invoke(proj, *left)))
            {
                // (first > last  <=>  last < first ) ++ and reverse
                do { ++left; ++right; } 
                while (right != last && std::invoke(comp, std::invoke(proj, *right), std::invoke(proj, *left)));
                std::ranges::reverse(first, right);
            }
            else
            {
                // while first <= last ++
                do { ++left; ++right; } 
                while (right != last && !std::invoke(comp, std::invoke(proj, *right), std::invoke(proj, *left)));
            }
            return right;
        }

        template <typename T>
        constexpr static T min_run_length(T n) 
        {
            T r = 0;      
            while (n >= MinSize) 
            {
                r |= (n & 1);
                n >>= 1;
            }
            return n + r;
        }

        template <typename T>
        constexpr static void merge_collapse(std::vector<T>& runs)
        {
            while (runs.size() > 2)
            {
                if (runs.size() == 3)
                {
                    // only have two runs
                    auto left = runs[0];
                    auto middle = runs[1];
                    auto right = runs[2];
                    if (middle - left <= right - middle)
                    {
                        std::inplace_merge(left, middle, right);
                        runs.erase(runs.begin() + 1); // remove middle
                    }
                    else 
                        break;
                }
                else
                {
                    auto last_pos = runs.end();
                    auto iter4 = *(last_pos - 1);
                    auto iter3 = *(last_pos - 2);
                    auto iter2 = *(last_pos - 3);
                    auto iter1 = *(last_pos - 4);
                    const auto Z = std::distance(iter1, iter2);
                    const auto Y = std::distance(iter2, iter3);
                    const auto X = std::distance(iter3, iter4);
                    if (X + Y < Z && X < Y)
                        break;
                    else
                    {
                        if (Z < X) // merge first 2
                        {
                            std::inplace_merge(iter1, iter2, iter3);
                            runs.erase(last_pos - 3);
                        }
                        else // merge last 2
                        {
                            std::inplace_merge(iter2, iter3, iter4);
                            runs.erase(last_pos - 2);
                        }
                    }
                }
            }
        }

        template <typename T>
        constexpr static void merge_force_collapse(std::vector<T>& runs)
        {
            if (runs.size() == 2)
                return;
            while (runs.size() > 2)
            {
                int last = runs.size();
                std::inplace_merge(runs[last - 3], runs[last - 2], runs[last - 1]);
                runs.erase(runs.end() - 2);
            }
        }
    
    public:
        template <std::random_access_iterator I, std::sentinel_for<I> S, 
            typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
        constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const
        {
            if (last - first < MinSize)
                return insertion_sort(std::move(first), std::move(last), std::move(comp), std::move(proj));


            auto iter = first; // 
            std::vector stack{ iter };

            // min length
            auto remaining = std::ranges::distance(iter, last);
            const auto min_run = min_run_length(remaining);
            do 
            {
                auto next_iter = count_run_and_make_ascending(iter, last, std::ref(comp), std::ref(proj));
                if (next_iter - iter < min_run)
                {
                    const auto dist = last - iter;
                    auto force = std::min(dist, min_run);
                    next_iter = insertion_sort(iter, iter + force, comp, proj);
                }
                stack.emplace_back(next_iter);
                merge_collapse(stack);
                iter = next_iter;

            } while (iter != last);

            merge_force_collapse(stack);

            return iter;
        }

        template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj>
        constexpr std::ranges::borrowed_iterator_t<Range>
        operator()(Range&& r, Comp comp = {}, Proj proj = {}) const
        {
            return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
        }

    };

    inline constexpr tim_sort_fn tim_sort{ };

    template <typename InIt, typename OutIt, typename T, typename F>
    InIt split(InIt it, InIt end_it, OutIt out_it, T split_val, F bin_func) 
    {
        using namespace std;
        while (it != end_it) 
        {
            auto slice_end(find(it, end_it, split_val));
            *out_it++ = bin_func(it, slice_end);
            if (slice_end == end_it) 
            {
                return end_it;
            }
            it = next(slice_end);
        }
        return it;
    }




/*

template <typename T>
auto map(T fn) {
    return [=](auto reduce_fn) {
        return [=] (auto accum, auto input) {
            return reduce_fn(accum, fn(input));
        };
    };
}

template <typename T>
auto filter(T predicate) {
    return [=](auto reduce_fn) {
        return [=](auto accume, auto input) {
            if (predicate(input)) {
                return reduce_fn(accume, input);
            } else {
                return accume;
            }
        };
    };
}


    std::istream_iterator<int> it{ std::cin };
    std::istream_iterator<int> end_it;
    auto even = [](int x) { return ~x & 1; };
    auto twice = [](int x) { return x << 1; };
    auto copy_and_advance = [](auto iter, auto input) {
        *iter = input;
        return ++iter;
    };
	// very ugly
    std::accumulate(it, end_it, std::ostream_iterator<int>{ std::cout, ", " }, 
        filter(even) (
            map(twice) (
                copy_and_advance
            )
        ));
    std::cout << std::endl;
*/

}  // namespace leviathan

#endif