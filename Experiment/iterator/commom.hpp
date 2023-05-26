// #pragma once

// #include <compare>
// #include <concepts>
// #include <functional>

// template <typename T, template <typename...> typename Primary>
// struct is_specialization_of : std::false_type { };

// template <template <typename...> typename Primary, typename... Args>
// struct is_specialization_of<Primary<Args...>, Primary> : std::true_type { };

// template <typename T, template <typename...> typename Primary>
// inline constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;

// template <typename T> 
// using with_reference = T&;

// template <typename T> 
// concept can_reference = requires { typename with_reference<T>; };

// template <typename T> 
// concept dereferenceable = requires(T& t) { { *t } -> can_reference; };

// template <dereferenceable T>
// using iter_reference_t = decltype(*std::declval<T&>());

// template <typename> struct incrementable_traits { };

// //  Incrementable traits
// template <typename T> requires std::is_object_v<T>
// struct incrementable_traits<T*>
// { using difference_type = ptrdiff_t; };

// template <typename I> 
// struct incrementable_traits<const I> : incrementable_traits<I> { };

// template <typename T>  requires requires { typename T::difference_type; }
// struct incrementable_traits<T>
// { using difference_type = typename T::difference_type; };

// template <typename T> 
//     requires (!requires { typename T::difference_type; } && 
//         requires (const T& a, const T& b) { { a - b } -> std::integral; })
// struct incrementable_traits<T>
// { using difference_type = std::make_signed_t<decltype(std::declval<T>() - std::declval<T>())>; };

// template <typename T> struct iter_difference : 
//     std::type_identity<typename std::iterator_traits<T>::difference_type> { };

// // FIXME: if iterator_traits<Ri> names a specialization generated from the primary template.
// template <typename T> requires requires { typename incrementable_traits<T>::difference_type; }
// struct iter_difference<T> : std::type_identity<typename incrementable_traits<T>::difference_type> { };

// template <typename T>
// using iter_difference_t = typename iter_difference<std::remove_cvref_t<T>>::type;

// //  Indirectly readable traits
// template <typename> struct cond_value_type { };

// template <typename T> requires std::is_object_v<T>
// struct cond_value_type<T> 
// { using value_type = std::remove_cv_t<T>; };

// template <typename T> concept has_member_value_type 
//     = requires { typename T::value_type; };

// template <typename T> concept has_member_element_type 
//     = requires { typename T::element_value; };

// template <typename> struct indirectly_readable_traits { };

// template <typename T> 
// struct indirectly_readable_traits<T*> : cond_value_type<T> { };

// template <typename T> requires std::is_array_v<T>
// struct indirectly_readable_traits<T> 
// { using value_type = std::remove_cv_t<std::remove_extent_t<T>>; };

// template <typename T> 
// struct indirectly_readable_traits<const T> 
//     : indirectly_readable_traits<T> { };

// template <has_member_value_type T> 
// struct indirectly_readable_traits<T> 
//     : cond_value_type<typename T::value_type> { };

// template <has_member_element_type T> 
// struct indirectly_readable_traits<T> 
//     : cond_value_type<typename T::element_type> { };

// // conflict, both has member value type and element type.
// template <has_member_value_type T> 
//     requires has_member_element_type<T>
// struct indirectly_readable_traits<T> { };

// // not conflict, since member value type and element type are same type.
// template <has_member_value_type T> 
//     requires has_member_element_type<T> && 
//         std::same_as<std::remove_cv_t<typename T::value_type>, std::remove_cv_t<typename T::element_type>>
// struct indirectly_readable_traits<T> 
//     : cond_value_type<typename T::value_type> { };

// template <typename T>
// struct iter_value : std::type_identity<typename std::iterator_traits<T>::value_type> { };

// // FIXME: if iterator_traits<Ri> names a specialization generated from the primary template.
// template <typename T> requires requires { typename indirectly_readable_traits<T>::value_type; }
// struct iter_value<T> : std::type_identity<typename indirectly_readable_traits<T>::value_type> { };

// template <typename T>
// using iter_value_t = typename iter_value<std::remove_cvref_t<T>>::type;

// namespace ranges
// {
//     inline namespace detail 
//     {
//         struct iter_move_fn 
//         {
//             auto&& operator()(auto&& x) 
//             { return std::move(*x); }    
//         };
    
//         inline constexpr iter_move_fn iter_move;
    
//         struct iter_swap_fn 
//         {
            
//         };
    
//         inline constexpr iter_swap_fn iter_swap;
//     }
// }

// template <dereferenceable T>
//     requires requires(T& t) { ranges::iter_move(t) -> can_reference; }
// using iter_rvalue_reference_t = decltype(ranges::iter_move(std::declval<T&>()));

// template <typename In>
// concept indirectly_readable_impl = 
//     requires (const In in) 
//     {
//         typename iter_value_t<In>;
//         typename iter_reference_t<In>;
//         typename iter_rvalue_reference_t<In>;
//         { *in } -> std::same_as<iter_reference_t<In>>;
//         { ranges::iter_move(in) } -> std::same_as<iter_rvalue_reference_t<In>>;
//     } && 
//     std::common_reference_with<iter_reference_t<In>&&, iter_value_t<In>&> && 
//     std::common_reference_with<iter_reference_t<In>&&, iter_rvalue_reference_t<In>&&> && 
//     std::common_reference_with<iter_rvalue_reference_t<In>&&, const iter_value_t<In>&>;

// template <typename In>
// concept indirectly_readable = indirectly_readable_impl<std::remove_cvref_t<In>>;

// template <typename Out, typename T>
// concept indirectly_writeable = 
//     requires (Out&& o, T&& t) 
//     {
//         *o = std::forward<T>(t);
//         *std::forward<Out>(o) = std::forward<T>(t);
//         const_cast<const iter_reference_t<Out>&&>(*o) = std::forward<T>(t);
//         const_cast<const iter_reference_t<Out>&&>(*std::forward<Out>(o)) = std::forward<T>(t);
//     };

// template <typename T> constexpr bool is_integer_like = std::integral<T>;
// template <typename T> constexpr bool is_signed_integer_like = std::signed_integral<T>;
// template <typename T> constexpr bool is_unsigned_integer_like = std::unsigned_integral<T>;

// // istream_iterator, a == b but ++a !=(may) ++b
// template <typename I>
// concept weakly_incrementable = 
//     std::movable<I> && 
//     requires (I i)
//     {
//         typename iter_difference_t<I>;
//         requires is_integer_like<iter_difference_t<I>>;
//         { ++i } -> std::same_as<I&>;
//         i++; 
//     };

// // equality-preserving
// // if bool(a == b) then bool(a++ == b)
// // if bool(a == b) then bool(((void)a++), a == ++b)
// template <typename I>
// concept incrementable = 
//     std::regular<I> &&
//     weakly_incrementable<I> && 
//     requires (I i) { { i++ } -> std::same_as<I>; };

// template <typename I>
// concept input_or_output_iterator =
//     requires (I i) { { *i } -> can_reference; } && 
//     weakly_incrementable<I>;

// template <typename T, typename U>
// concept weakly_equality_comparable_with = 
//     requires (const std::remove_reference_t<T>& t, const std::remove_reference_t<U>& u)
//     {
//         // FIXME: boolean-testable 
//         { t == u } -> std::convertible_to<bool>;
//         { t != u } -> std::convertible_to<bool>;
//         { u == t } -> std::convertible_to<bool>;
//         { u != t } -> std::convertible_to<bool>;
//     };

// template <typename S, typename I>
// concept sentinel_for =
//     std::semiregular<S> &&
//     input_or_output_iterator<I> &&
//     weakly_equality_comparable_with<S, I>;

// template <typename S, typename I>
// concept sized_sentinel_for =
//     sentinel_for<S, I> &&
//     !std::disable_sized_sentinel_for<std::remove_cv_t<S>, std::remove_cv_t<I>> &&
//     requires (const I& i, const S& s) 
//     {
//         { s - i } -> std::same_as<iter_difference_t<I>>;
//         { i - s } -> std::same_as<iter_difference_t<I>>;
//     };

// template <typename T>
// concept has_iterator_concept = requires { typename T::iterator_concept; };

// template <typename T>
// concept has_iterator_category = requires { typename T::iterator_category; };

// template <typename T> struct iter_concept { };

// template <has_iterator_concept T> 
// struct iter_concept<T> : std::type_identity<typename T::iterator_concept> { };

// template <has_iterator_category T> requires (!has_iterator_concept<T>)
// struct iter_concept<T> : std::type_identity<typename T::iterator_category> { };

// template <typename T>
// using iter_concept_t = typename iter_concept<T>::type;

// template <typename I>
// concept input_iterator = 
//     input_or_output_iterator<I> && 
//     indirectly_readable<I> && 
//     requires { typename iter_concept_t<I>; } && 
//     std::derived_from<iter_concept_t<I>, std::input_iterator_tag>;

// template <typename I, typename T>
// concept output_iterator = 
//     input_or_output_iterator<I> && 
//     indirectly_writeable<I, T> &&
//     requires (I i, T&& t) { *i++ = std::forward<T>(t); };

// template <typename I> 
// concept forward_iterator =
//     std::input_iterator<I> && 
//     std::derived_from<iter_concept_t<I>, std::forward_iterator_tag> &&
//     incrementable<I> && 
//     sentinel_for<I, I>;

// template <typename I> 
// concept bidirectional_iterator = 
//     forward_iterator<I> && 
//     std::derived_from<iter_concept_t<I>, std::bidirectional_iterator_tag> && 
//     requires (I i) 
//     {
//         { --i } -> std::same_as<I&>;
//         { i-- } -> std::same_as<I>;
//     };

// template <typename I> 
// concept random_access_iterator = 
//     bidirectional_iterator<I> && 
//     std::derived_from<iter_concept_t<I>, std::random_access_iterator_tag> && 
//     std::totally_ordered<I> &&
//     sized_sentinel_for<I, I> && 
//     requires (I i, const I j, const iter_difference_t<I> n)
//     {
//         { i += n } -> std::same_as<I&>;
//         { j +  n } -> std::same_as<I>;
//         { n +  j } -> std::same_as<I>;
//         { i -= n } -> std::same_as<I&>;
//         { j -  n } -> std::same_as<I>;
//         {  j[n]  } -> std::same_as<iter_reference_t<I>; 
//     };

// template <typename I> 
// concept contiguous_iterator = 
//     random_access_iterator<I> && 
//     std::derived_from<iter_concept_t<I>, std::contiguous_iterator_tag> && 
//     std::is_lvalue_reference_v<iter_reference_t<I>> && 
//     std::same_as<iter_value_t<I>, std::remove_cvref_t<iter_reference_t<I>>> && 
//     requires (const I& i) { { std::to_address(i) } -> std::same_as<std::add_pointer_t<iter_reference_t<I>>>; };

// template <typename T>
// struct indirect_value;

// template <typename T>
// using indirect_value_t = typename indirect_value<T>::type;

// template <indirectly_readable T>
// using iter_common_reference_t = std::common_reference_t<iter_reference_t<T>, indirectly_value_t<T>>;

// template <typename F, typename I>
// concept indirectly_unary_invocable = 
//     indirectly_readable<I> &&
//     std::copy_constructible<F> && 
//     std::invocable<F&, indirect_value_t<I>> &&
//     std::invocable<F&, iter_reference_t<I>> &&
//     std::invocable<F&, iter_common_reference_t<I>> &&
//     std::common_reference_with<
//         std::invoke_result_t<F&, indirect_value_t<I>>,
//         std::invoke_result_t<F&, iter_reference_t<I>>>;

// template <typename F, typename I>
// concept indirectly_regular_unary_invocable = 
//     indirectly_readable<I> && 
//     std::copy_constructible<F> && 
//     std::regular_invocable<F&, indirect_value_t<I>> &&
//     std::regular_invocable<F&, iter_reference_t<I>> &&
//     std::regular_invocable<F&, iter_common_reference_t<I>> &&
//     std::common_reference_with<
//         std::invoke_result_t<F&, indirect_value_t<I>>,
//         std::invoke_result_t<F&, iter_reference_t<I>>>;
        
// template <typename T>
// struct indirect_value : std::type_identity<iter_value_t<T>> { };

// // template <indirectly_readable I, indirectly_regular_unary_invocable<I> Proj> 
// // struct projected;




// // template <typename T>
// // struct indirect_value : std::type_identity<iter_value_t<T>> { };

// // template <typename T, typename Proj> 
// // struct indirect_value<protected<T, Proj> 
// //     : std::type_identity<std::invoke_result_t<>> { };

// // template <typename F, typename I>
// // concept indirectly_regular_unary_invocable;

// // template <typename F, typename I>
// // concept indirect_unary_predicate;

// // template <typename F, typename I1, typename I2>
// // concept indirect_binary_predicate;

// // template <typename F, typename I1, typename I2 = I1>
// // concept indirect_equivalence_relation;

// // template <typename F, typename I1, typename I2 = I1>
// // concept indirect_strict_weak_order;

// // template <typename F, typename... Is>
// //     requires (indirectly_readable<Is> &&...) && std::invocable<F, iter_reference_t<Is>...>
// // using indirect_result_t = std::invoke_result_t<F, iter_reference_t<Is...>>;



// // template <indirectly_readable I, typename Proj>
// // struct incrementable_traits<projected<I, Proj>>;

// // template <typename In, typename Out> concept indirectly_moveable;

// // template <typename In, typename Out> concept indirectly_moveable_storable;

// // template <typename In, typename Out> concept indirectly_copyable;

// // template <typename In, typename Out> concept indirectly_copyable_storable;

// // template <typename I1, typename I2> concept indirectly_swappable;

// // template <typename I1, typename I2, typename R, typename P1 = std::identity, typename P2 = std::identity> concept indirectly_comparable;

// // template <typename I>
// // concept permutable;

// // template <typename I1, typename I2, typename Out, typename R = std::ranges::less, typename P1 = std::identity, typename P2 = std::identity>
// // concept mergeable;

// // template <typename I, typename R = std::ranges::less, typename P = std::identity>
// // concept sortable;






