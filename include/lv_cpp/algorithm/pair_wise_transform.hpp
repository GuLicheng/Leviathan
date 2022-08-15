#pragma once

#include "common.hpp"

namespace leviathan
{

    /*
        https://en.cppreference.com/w/cpp/algorithm/ranges/transform
        Return:
            a unary_transform_result contains an input iterator equal to the last element not transformed
            and an output iterator to the element past the last element transformed.

        e.g.
            std::vector<int> v = { 0, 1, 2, 3, 4 }, dest;
            auto [ret, _] = pair_wise_stride_transform(v.begin(), v.end(), std::back_inserter(dest), std::plus<>());
            assert(dest == { 1, 5 });
            assert(std::distance(ret, v.end()) == 1);
            assert(*ret == 4);
    */
    struct pair_wise_stride_transform_fn
    {
        template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O, std::copy_constructible F, typename Proj = std::identity>
        requires std::indirectly_writable<O, std::indirect_result_t<F&, std::projected<I, Proj>, std::projected<I, Proj>>>
        constexpr std::ranges::unary_transform_result<I, O> operator() (I first, S last, O result, F op, Proj proj = {}) const
        {
            auto second = std::ranges::next(first, 1, last);

            // at least two elements
            for (; second != last;)
            {
                *result++ = std::invoke(op, std::invoke(proj, *first), std::invoke(proj, *second));
                std::ranges::advance(second, 2, last);
                std::ranges::advance(first, 2, last);
            }
            return { first, result };
        }

        template <std::ranges::input_range R, std::weakly_incrementable O, std::copy_constructible F, typename Proj = std::identity>
        requires std::indirectly_writable<O, std::indirect_result_t<F&, std::projected<std::ranges::iterator_t<R>, Proj>, std::projected<std::ranges::iterator_t<R>, Proj>>>
        constexpr std::ranges::unary_transform_result<std::ranges::iterator_t<R>, O> operator() (R&& r, O result, F op, Proj proj = {}) const
        {
            return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(result), std::move(op), std::move(proj));
        }

    };

    inline constexpr pair_wise_stride_transform_fn pair_wise_stride_transform{};


    /*
        Return:
            a unary_transform_result contains an input iterator equal to the last
            and an output iterator to the element past the last element transformed.

        e.g.
            std::vector<int> v = { 0, 1, 2, 3, 4 }, dest;
            auto [ret, _] = pair_wise_stride_transform(v.begin(), v.end(), std::back_inserter(dest), std::plus<>());
            assert(dest == { 1, 3, 5, 7 });
    */
    struct pair_wise_transform_fn 
    {
        template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O, std::copy_constructible F, typename Proj = std::identity>
        requires std::indirectly_writable<O, std::indirect_result_t<F &, std::projected<I, Proj>, std::projected<I, Proj>>>
        constexpr std::ranges::unary_transform_result<I, O> operator()(I first, S last, O result, F op, Proj proj = {}) const
        {
            auto second = std::ranges::next(first, 1, last);

            for (; second != last; ++first, ++second)
                *result++ = std::invoke(op, std::invoke(proj, *first), std::invoke(proj, *second));

            return { second, result };
        }

        template <std::ranges::input_range R, std::weakly_incrementable O, std::copy_constructible F, typename Proj = std::identity>
        requires std::indirectly_writable<O, std::indirect_result_t<F&, std::projected<std::ranges::iterator_t<R>, Proj>, std::projected<std::ranges::iterator_t<R>, Proj>>>
        constexpr std::ranges::unary_transform_result<std::ranges::iterator_t<R>, O> operator() (R&& r, O result, F op, Proj proj = {}) const
        {
            return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(result), std::move(op), std::move(proj));
        }

    };

    inline constexpr pair_wise_transform_fn pair_wise_transform{};


} // namespace leviathan
