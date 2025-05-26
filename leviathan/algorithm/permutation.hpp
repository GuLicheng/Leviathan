// // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2639.pdf
// // FIXME: This is not correct
// #pragma once

// #include "common.hpp"

// namespace cpp::ranges
// {

// inline constexpr struct 
// {
//     template <std::bidirectional_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
//         requires std::sortable<I, Comp, Proj>
//     static constexpr auto operator()(I first, I middle, S last, Comp comp = {}, Proj proj = {}) 
//     {
//         std::ranges::reverse(middle, last);
//         return std::ranges::next_permutation(first, last, std::move(comp), std::move(proj));
//     } 

//     template <std::ranges::bidirectional_range R, typename Comp = std::ranges::less, typename Proj = std::identity>
//         requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
//     static constexpr auto operator()(R&& r, std::ranges::iterator_t<R> middle, Comp comp = {}, Proj proj = {}) 
//     {
//         return operator()(std::ranges::begin(r), std::move(middle), std::ranges::end(r),
//                 std::move(comp), std::move(proj));
//     }

// } next_partial_permutation;

// inline constexpr struct 
// {
//     template <std::bidirectional_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
//         requires std::sortable<I, Comp, Proj>
//     static constexpr auto operator()(I first, I middle, S last, Comp comp = {}, Proj proj = {}) 
//     {
//         auto result = std::ranges::prev_permutation(first, last, std::move(comp), std::move(proj));
//         std::ranges::reverse(middle, last);
//         return result;
//     } 

//     template <std::ranges::bidirectional_range R, typename Comp = std::ranges::less, typename Proj = std::identity>
//         requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
//     static constexpr auto operator()(R&& r, std::ranges::iterator_t<R> middle, Comp comp = {}, Proj proj = {}) 
//     {
//         return operator()(std::ranges::begin(r), std::move(middle), std::ranges::end(r),
//                 std::move(comp), std::move(proj));
//     }

// } prev_partial_permutation;

// }

