// #pragma once

// #include "common.hpp"

// // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2441r2.html
// namespace leviathan::ranges
// {
//     template <typename R, typename P>
//     concept compatible_joinable_ranges = 
//         std::common_with<std::ranges::range_value_t<R>, std::ranges::range_value_t<P>> && 
//         std::common_reference_with<std::ranges::range_reference_t<R>, std::ranges::range_reference_t<P>> && 
//         std::common_reference_with<std::ranges::range_rvalue_reference_t<R>, std::ranges::range_rvalue_reference_t<P>>;


//     template <typename R>
//     concept bidirectional_common = std::ranges::bidirectional_range<R> && std::ranges::common_range<R>;

//     struct empty_class { }; //  an empty class for some case that non_propagating_cache is not exist

//     template <std::ranges::input_range V, std::ranges::forward_range Pattern>
//     requires std::ranges::view<V> 
//             && std::ranges::input_range<std::ranges::range_reference_t<V>> 
//             && std::ranges::view<Pattern> 
//             && compatible_joinable_ranges<std::ranges::range_reference_t<V>, Pattern>
//     class join_with_view : public std::ranges::view_interface<join_with_view<V, Pattern>>
//     {
//         using inner_range = std::ranges::range_reference_t<V>;
//         constexpr static bool has_inner_non_propagating_cache = !std::is_reference_v<inner_range>;
//         using non_propagating_cache = std::optional<std::remove_cv_t<inner_range>>;

//         V m_base = V();
//         Pattern m_pattern = Pattern(); 

//         [[no_unique_address]] std::conditional_t<has_inner_non_propagating_cache, non_propagating_cache, empty_class> m_inner;
    
//         template <bool Const> struct iterator;
//         template <bool Const> struct sentinel;

//     public:
//         join_with_view() requires std::default_initializable<V> && std::default_initializable<Pattern> = default;

//         constexpr join_with_view(V base, Pattern pattern) : m_base(std::move(base)), m_pattern(std::move(pattern)) { }

//         template <std::ranges::input_range R>
//         requires std::constructible_from<V, std::views::all_t<R>> && std::constructible_from<Pattern, std::ranges::single_view<std::ranges::range_value_t<inner_range>>>
//         constexpr join_with_view(R&& r, std::ranges::range_value_t<inner_range> e) : m_base(std::views::all(std::forward<R>(r))), m_pattern(std::views::single(std::move(e))) { }

//         constexpr V base() const& requires std::copy_constructible<V> 
//         { return m_base; }

//         constexpr V base() && 
//         { return std::move(m_base); }

//         constexpr auto begin()
//         {
//             constexpr bool use_const = detail::simple_view<V> && detail::simple_view<Pattern> && std::is_reference_v<inner_range>;
//             return iterator<use_const>(*this, std::ranges::begin(m_base));
//         }

//         constexpr auto begin() const requires std::ranges::input_range<const V> && std::ranges::forward_range<const Pattern> && std::is_reference_v<std::ranges::range_reference_t<const V>>
//         {
//             return iterator<true>(*this, std::ranges::begin(m_base));
//         }

//         constexpr auto end() 
//         {
//             constexpr bool use_const = detail::simple_view<V> && detail::simple_view<Pattern>;
//             if constexpr (std::ranges::forward_range<V> && std::is_reference_v<inner_range> && std::ranges::forward_range<inner_range> && std::ranges::common_range<V> && std::ranges::common_range<inner_range>)
//             {
//                 return iterator<use_const>(*this, std::ranges::end(m_base));
//             }
//             else    
//             {
//                 return sentinel<use_const>(*this);
//             }
//         }

//         constexpr auto end() const requires std::ranges::input_range<const V> && std::ranges::forward_range<const Pattern> && std::is_reference_v<std::ranges::range_reference_t<const V>>
//         {
//             using inner_const_range = std::ranges::range_reference_t<const V>;
//             if constexpr (std::ranges::forward_range<const V> && std::ranges::forward_range<inner_const_range> && std::ranges::common_range<const V> && std::ranges::common_range<const Pattern>)
//             {
//                 return iterator<true>(*this, std::ranges::end(m_base));
//             }
//             else
//             {
//                 return sentinel<true>(*this);
//             }
//         }

//     };

//     template <typename R, typename P>
//     join_with_view(R&&, P&&) -> join_with_view<std::views::all_t<R>, std::views::all_t<P>>;

//     template <std::ranges::input_range R>
//     join_with_view(R&&, std::ranges::range_value_t<std::ranges::range_reference_t<R>>) -> join_with_view<std::views::all_t<R>, std::ranges::single_view<std::ranges::range_value_t<std::ranges::range_reference_t<R>>>>;

//     template <bool Const, typename V, typename Pattern>
//     struct join_with_view_iterator_category
//     {
//         using base = detail::maybe_const_t<Const, V>;                    
//         using inner_base = std::ranges::range_reference_t<base>;         
//         using pattern_base = detail::maybe_const_t<Const, Pattern>;      
//         using outer_iterator = std::ranges::iterator_t<base>;            
//         using inner_iterator = std::ranges::iterator_t<inner_base>;      
//         using pattern_iterator = std::ranges::iterator_t<pattern_base>;  
//         static constexpr bool ref_is_glvalue = std::is_reference_v<inner_base>;

//         using OUTERC = detail::iter_category_t<outer_iterator>;
//         using INNERC = detail::iter_category_t<inner_iterator>;
//         using PATTERNC = detail::iter_category_t<pattern_iterator>;

//         constexpr static auto category() 
//         {
//             if constexpr (std::is_lvalue_reference_v<std::common_reference_t<std::iter_reference_t<inner_iterator>, std::iter_reference_t<pattern_iterator>>>)
//                 return std::input_iterator_tag();
//             else if constexpr (std::derived_from<OUTERC, std::bidirectional_iterator_tag> && std::derived_from<INNERC, std::bidirectional_iterator_tag> && std::derived_from<PATTERNC, std::bidirectional_iterator_tag> && std::ranges::common_range<inner_base> && std::ranges::common_range<pattern_base>)
//                 return std::bidirectional_iterator_tag();
//             else if constexpr (std::derived_from<OUTERC, std::forward_iterator_tag> && std::derived_from<INNERC, std::forward_iterator_tag> && std::derived_from<PATTERNC, std::forward_iterator_tag>)
//                 return std::forward_iterator_tag();
//             else 
//                 return std::input_iterator_tag();
//         }

//         using type = decltype(category());

//     };

//     template <std::ranges::input_range V, std::ranges::forward_range Pattern>
//     requires std::ranges::view<V> && std::ranges::input_range<std::ranges::range_reference_t<V>> && std::ranges::view<Pattern> && compatible_joinable_ranges<V, Pattern>
//     template <bool Const>
//     struct join_with_view<V, Pattern>::iterator : public detail::has_typedef_name_of_iterator_category<std::is_reference_v<std::ranges::range_reference_t<detail::maybe_const_t<Const, V>>> && std::ranges::forward_range<detail::maybe_const_t<Const, V>> && std::ranges::forward_range<std::ranges::range_reference_t<detail::maybe_const_t<Const, V>>>, typename join_with_view_iterator_category<Const, V, Pattern>::type>
//     {

//         // consider std::vector<std::string> V = { "hello", "world", "!" }, pattern = "-"/'-';

//         using parent = detail::maybe_const_t<Const, join_with_view>;

//         using base = detail::maybe_const_t<Const, V>;                    // std::vector
//         using inner_base = std::ranges::range_reference_t<base>;         // std::string
//         using pattern_base = detail::maybe_const_t<Const, Pattern>;      // const char[]

//         using outer_iterator = std::ranges::iterator_t<base>;            // std::vector::iterator
//         using inner_iterator = std::ranges::iterator_t<inner_base>;      // std::string::iterator
//         using pattern_iterator = std::ranges::iterator_t<pattern_base>;  // const char*

//         static constexpr bool ref_is_glvalue = std::is_reference_v<inner_base>;

//         parent* m_parent = nullptr;
//         outer_iterator m_outer_iter = outer_iterator();
//         std::variant<pattern_iterator, inner_iterator> m_inner_iter;

//         constexpr iterator(parent& p, std::ranges::iterator_t<base> outer)
//             : m_parent(std::addressof(p)), m_outer_iter(std::move(outer))
//         {
//             if (m_outer_iter != std::ranges::end(m_parent->m_base))
//             {
//                 auto&& inner = update_inner(m_outer_iter);
//                 m_inner_iter.template emplace<1>(std::ranges::begin(inner));
//                 satisfy();
//             }
//         }
        
//         constexpr auto&& update_inner(const outer_iterator& x)
//         {
//             if constexpr (ref_is_glvalue)
//                 return *x;
//             else    
//                 return m_parent->m_inner.emplace(*x);
//         }

//         constexpr auto&& get_inner(const outer_iterator& x)
//         {
//             if constexpr (ref_is_glvalue)
//                 return *x;
//             else 
//                 return *(m_parent->m_inner);
//         }

//         constexpr void satisfy()
//         {
//             while (1) 
//             {
//                 // pattern
//                 if (m_inner_iter.index() == 0)
//                 {
//                     if (std::get<0>(m_inner_iter) != std::ranges::end(m_parent->m_pattern))
//                         break;
//                     auto&& inner = update_inner(m_outer_iter);
//                     m_inner_iter.template emplace<1>(std::ranges::begin(inner));
//                 }
//                 else
//                 {
//                     auto&& inner = get_inner(m_outer_iter);
//                     if (std::get<1>(m_inner_iter) != std::ranges::end(inner))
//                         break;
//                     if (++m_outer_iter == std::ranges::end(m_parent->m_base))
//                     {
//                         if constexpr (ref_is_glvalue)
//                             m_inner_iter.template emplace<0>();
//                         break;
//                     }

//                     m_inner_iter.template emplace<0>(std::ranges::begin(m_parent->m_pattern));

//                 }
//             }
//         }

//         constexpr static auto iterator_concept_check() 
//         {
//             if constexpr (ref_is_glvalue && std::ranges::bidirectional_range<base> && bidirectional_common<inner_base> && bidirectional_common<pattern_base>)
//                 return std::bidirectional_iterator_tag();
//             else if constexpr (ref_is_glvalue && std::ranges::forward_range<base> && std::ranges::forward_range<inner_base>)
//                 return std::forward_iterator_tag();
//             else 
//                 return std::input_iterator_tag();
//         }

//     public:

//         using iterator_concept = decltype(iterator_concept_check());
//         using value_type = std::common_type_t<std::iter_value_t<inner_iterator>, std::iter_value_t<pattern_iterator>>;
//         using difference_type = std::common_type_t<std::iter_difference_t<outer_iterator>, std::iter_difference_t<inner_iterator>, std::iter_difference_t<pattern_iterator>>;;

//         iterator() requires std::default_initializable<outer_iterator> = default;
//         constexpr iterator(iterator<!Const> i) requires Const && std::convertible_to<std::ranges::iterator_t<V>, outer_iterator> && std::convertible_to<std::ranges::iterator_t<inner_range>, inner_iterator> && std::convertible_to<std::ranges::iterator_t<Pattern>, pattern_iterator>
//             : m_outer_iter(std::move(i.m_outer_iter)), m_parent(i.m_parent)
//         {
//             if (i.m_inner_iter.index() == 0)
//                 m_inner_iter.template emplace<0>(std::get<0>(std::move(i.m_inner_iter)));
//             else
//                 m_inner_iter.template emplace<1>(std::get<1>(std::move(i.m_inner_iter)));
//         }

//         constexpr decltype(auto) operator*() const
//         {
//             using reference = std::common_reference_t<std::iter_reference_t<inner_iterator>, std::iter_reference_t<pattern_iterator>>;
//             return std::visit([](auto& it) -> reference { return *it; }, m_inner_iter);
//         }

//         constexpr iterator& operator++() 
//         {
//             std::visit([](auto& it) { ++it; }, m_inner_iter);
//             satisfy();
//             return *this;
//         }

//         constexpr void operator++(int)
//         { (void)++*this; }

//         constexpr iterator operator++(int) requires ref_is_glvalue && std::forward_iterator<outer_iterator> && std::forward_iterator<inner_iterator>
//         {
//             auto temp = *this;
//             ++*this;
//             return temp;
//         }

        
//         constexpr iterator& operator--() requires ref_is_glvalue && std::ranges::bidirectional_range<base> && bidirectional_common<inner_base> && bidirectional_common<pattern_base>
//         {
//             if (m_outer_iter == std::ranges::end(m_parent->m_base))
//             {
//                 auto&& inner = *--m_outer_iter;
//                 m_inner_iter.template emplace<1>(std::ranges::end(inner));
//             }

//             while (1)
//             {
//                 if (m_inner_iter.index() == 0) 
//                 {
//                     auto& it = std::get<0>(m_inner_iter);
//                     if (it == std::ranges::begin(m_parent->m_pattern)) 
//                     {
//                         auto&& inner = *--m_outer_iter;
//                         m_inner_iter.template emplace<1>(std::ranges::end(inner));
//                     }
//                     else
//                         break;
//                 }
//                 else
//                 {
//                     auto& it = std::get<1>(m_inner_iter);
//                     auto&& inner = *m_outer_iter;
//                     if (it == std::ranges::begin(inner))
//                         m_inner_iter.template emplace<0>(std::ranges::end(m_parent->m_pattern));
//                     else   
//                         break;
//                 }
//             }

//             std::visit([](auto& it) { --it; }, m_inner_iter);
//             return *this;
//         }

//         constexpr iterator operator--(int) requires ref_is_glvalue && std::ranges::bidirectional_range<base> && bidirectional_common<inner_base> && bidirectional_common<pattern_base>
//         {
//             auto temp = *this;
//             --*this;
//             return temp;
//         }

//         friend constexpr bool operator==(const iterator& x, const iterator& y)
//         requires ref_is_glvalue && std::equality_comparable<outer_iterator> && std::equality_comparable<inner_iterator>
//         { return x.m_outer_iter == y.m_outer_iter && x.m_inner_iter == y.m_inner_iter; }

//         friend constexpr decltype(auto) iter_move(const iterator& x)
//         {
//             using rvalue_reference = std::common_reference_t<
//                 std::iter_rvalue_reference_t<inner_iterator>,
//                 std::iter_rvalue_reference_t<pattern_iterator>>;
//             return std::visit<rvalue_reference>(std::ranges::iter_move, x.m_inner_iter);
//         }

//         friend constexpr void iter_swap(const iterator& x, const iterator& y)
//         requires std::indirectly_swappable<inner_iterator, pattern_iterator>
//         { std::visit(std::ranges::iter_swap, x.m_inner_iter, y.m_inner_iter); }

//     };


//     template <std::ranges::input_range V, std::ranges::forward_range Pattern>
//     requires std::ranges::view<V> && std::ranges::input_range<std::ranges::range_reference_t<V>> && std::ranges::view<Pattern> && compatible_joinable_ranges<V, Pattern>
//     template <bool Const>
//     struct join_with_view<V, Pattern>::sentinel
//     {
//         using parent = detail::maybe_const_t<Const, join_with_view>;
//         using base = detail::maybe_const_t<Const, V>;    

//         std::ranges::sentinel_t<base> m_end = std::ranges::sentinel_t<base>();

//         constexpr explicit sentinel(parent& p) 
//             : m_end(std::ranges::end(p.m_base)) { }

//     public:
//         sentinel() = default;
//         constexpr sentinel(sentinel<!Const> s) requires Const && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<base>>
//             : m_end(std::move(s.m_end)) { }
    
//         template <bool OtherConst>
//         requires std::sentinel_for<std::ranges::sentinel_t<base>, std::ranges::iterator_t<detail::maybe_const_t<OtherConst, V>>>
//         friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
//         { return x.m_outer_iter == y.m_end; }
    
//     };


//     struct join_with_adaptor : range_adaptor<join_with_adaptor>
//     {
//         using range_adaptor<join_with_adaptor>::operator();

//         template <std::ranges::viewable_range V, typename Pattern>
//         requires requires { join_with_view(std::declval<V>(), std::declval<Pattern>()); }
//         constexpr auto [[nodiscard]] operator()(V&& v, Pattern&& p) const
//         { return join_with_view{ (V&&)v, (Pattern&&)p }; }

//     };

//     inline constexpr join_with_adaptor join_with{};

// }
