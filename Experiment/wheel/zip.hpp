// #pragma once

// #include "common.hpp"

// #include <algorithm>

// // range.zip_view
// namespace leviathan::ranges
// {
//     template <typename... Rs>
//     concept zip_is_common = (sizeof...(Rs) == 1 && (std::ranges::common_range<Rs> && ...)) || 
//         (!(std::ranges::bidirectional_range<Rs> && ...) && (std::ranges::common_range<Rs> && ...)) || 
//         ((std::ranges::random_access_range<Rs> && ...) && (std::ranges::sized_range<Rs> && ...));


//     template <bool Const, typename... Vs>
//     concept all_bidirectional = (std::ranges::bidirectional_range<detail::maybe_const_t<Const, Vs>> && ...);

//     template <bool Const, typename... Vs>
//     concept all_random_access = (std::ranges::random_access_range<detail::maybe_const_t<Const, Vs>> && ...);

//     template <bool Const, typename... Vs>
//     concept all_forward = (std::ranges::forward_range<detail::maybe_const_t<Const, Vs>> && ...);

//     // tuple-or-pair
//     // tuple-transform
//     // tuple-foreach

//     template <std::ranges::input_range... Vs>
//         requires (std::ranges::view<Vs> && ...) && (sizeof...(Vs) > 0)
//     class zip_view : public std::ranges::view_interface<zip_view<Vs...>>
//     {
//         std::tuple<Vs...> m_views;

//         template <bool> struct iterator;
//         template <bool> struct sentinel;

//         template <bool Const> struct iterator 
//             : detail::has_typedef_name_of_iterator_category<all_forward<Const, Vs...>, std::input_iterator_tag>
//         {
//             static constexpr bool is_all_random_access = all_random_access<Const, Vs...>;
//             using tp_t = detail::tuple_or_pair<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>...>;
//             tp_t m_current;
//             constexpr explicit iterator(tp_t current) : m_current(std::move(current)) { }
//         public:

//             using iterator_concept = decltype([]{
//                 if constexpr (is_all_random_access)
//                     return std::random_access_iterator_tag();
//                 else if constexpr (all_bidirectional<Const, Vs...>)
//                     return std::bidirectional_iterator_tag();
//                 else if constexpr (all_forward<Const, Vs...>)
//                     return std::forward_iterator_tag();
//                 else    
//                     return std::input_iterator_tag();
//             }());

//             using value_type = detail::tuple_or_pair<std::ranges::range_value_t<detail::maybe_const_t<Const, Vs>>...>;
//             using difference_type = std::common_type_t<std::ranges::range_difference_t<detail::maybe_const_t<Const, Vs>>...>;

//             iterator() = default;
//             constexpr iterator(iterator<!Const> i)
//                 requires Const && (std::convertible_to<std::ranges::iterator_t<Vs>, std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
//                     : m_current(std::move(i.m_current)) { }

//             constexpr auto operator*() const
//             {
//                 return detail::tuple_transform([](auto& i) -> decltype(auto) { 
//                     return *i; 
//                 }, m_current);
//             }

//             constexpr iterator& operator++()
//             {
//                 detail::tuple_for_each([](auto& i) { ++i; }, m_current);
//                 return *this;
//             }
            
//             constexpr iterator operator++(int) requires all_forward<Const, Vs...>
//             {
//                 auto temp = *this;
//                 ++*this;
//                 return temp;
//             }

//             constexpr void operator++(int) 
//             {
//                 (void)(++*this);
//             }

//             constexpr iterator& operator--() requires all_bidirectional<Const, Vs...>
//             {
//                 detail::tuple_for_each([](auto& i) { --i; }, m_current);
//                 return *this;
//             }

//             constexpr iterator operator--(int) requires all_bidirectional<Const, Vs...>
//             {
//                 auto temp = *this;
//                 --*this;
//                 return temp;
//             }

//             constexpr iterator& operator+=(difference_type n)  
//                 requires all_random_access<Const, Vs...>
//             {
//                 detail::tuple_for_each([&]<typename I>(I& i) { i += std::iter_difference_t<I>(n); }, m_current);
//                 return *this;
//             }

//             constexpr iterator& operator-=(difference_type n)  
//                 requires all_random_access<Const, Vs...>
//             {
//                 detail::tuple_for_each([&]<typename I>(I& i) { i -= std::iter_difference_t<I>(n); }, m_current);
//                 return *this;
//             }

//             constexpr auto operator[](difference_type n) const 
//                 requires all_random_access<Const, Vs...>
//             {
//                 return detail::tuple_transform([&]<typename I>(I& i) -> decltype(auto) {
//                     return i[std::iter_difference_t<I>(n)];
//                 }, m_current);
//             }

//             friend constexpr bool operator==(const iterator& x, const iterator& y)
//                 requires (std::equality_comparable<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
//             {
//                 if constexpr (all_bidirectional<Const, Vs...>)
//                     return x.m_current == y.m_current;
//                 else
//                 {
//                     return [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
//                         return (static_cast<bool>(std::get<Idx>(x.m_current) == std::get<Idx>(y.m_current)) || ...);
//                     }(std::index_sequence_for<Vs...>());
//                 }
//             }

//             friend constexpr bool operator<(const iterator& x, const iterator& y) requires is_all_random_access
//             {
//                 return x.m_current < y.m_current;
//             }

//             friend constexpr bool operator<=(const iterator& x, const iterator& y) requires is_all_random_access
//             {
//                 return !(y < x);
//             }
//             friend constexpr bool operator>(const iterator& x, const iterator& y) requires is_all_random_access
//             {
//                 return y < x;
//             }
//             friend constexpr bool operator>=(const iterator& x, const iterator& y) requires is_all_random_access
//             {
//                 return !(x < y);
//             }

//             friend constexpr bool operator<=>(const iterator& x, const iterator& y) 
//                 requires is_all_random_access && (std::three_way_comparable<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
//             {
//                 return x <=> y;   
//             }

//             friend constexpr iterator operator+(const iterator& i, difference_type n)
//                 requires is_all_random_access
//             {
//                 auto r = i; 
//                 r += n;
//                 return r;
//             }
//             friend constexpr iterator operator+(difference_type n, const iterator& i)
//                 requires is_all_random_access
//             {
//                 return i + n;
//             }

//             friend constexpr iterator operator-(const iterator& i, difference_type n)
//                 requires is_all_random_access
//             {
//                 auto r = i; 
//                 r -= n;
//                 return r;
//             }
            
//             friend constexpr difference_type operator-(const iterator& x, const iterator& y)
//                 requires (std::sized_sentinel_for<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>,
//                                                   std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
//             {
//                 // Let D be the return type. Let DIST (i) be D(std::get<i>(x.current_) - std::get<i>(y.end_)).
//                 // The value with the smallest absolute value among DIST (n) for all integers 0 ≤ n < sizeof...(Views)
//                 [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
//                     difference_type D[] = { difference_type(std::get<Idx>(x.m_current) - std::get<Idx>(y.m_current))... };
//                     return std::ranges::min(D);
//                 }(std::index_sequence_for<Vs...>());
//             }
            

//             friend constexpr auto iter_move(const iterator& i) 
//                 noexcept((noexcept(std::ranges::iter_move(std::declval<const std::ranges::iterator_t<detail::maybe_const_t<Const,
//                 Vs>>&>())) && ...) &&
//                 (std::is_nothrow_move_constructible_v<std::ranges::range_rvalue_reference_t<detail::maybe_const_t<Const,
//                 Vs>>> && ...))
//             {
//                 return detail::tuple_transform(std::ranges::iter_move, i.m_current);
//             }

//             // FIXME: noexcept
//             friend constexpr auto iter_swap(const iterator& l, const iterator& r) noexcept(true)
//                 requires (std::indirectly_swappable<std::ranges::iterator_t<detail::maybe_const_t<Const, Vs>>> && ...)
//             {
//                 [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
//                     (std::ranges::iter_swap(std::get<Idx>(l.m_current), std::get<Idx>(r.m_current)), ...);
//                 }(std::index_sequence_for<Vs...>());
//             }

//         };

//         template <bool Const>
//         struct sentinel
//         {
//             detail::tuple_or_pair<std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>...> m_end;

//             constexpr explicit sentinel(detail::tuple_or_pair<std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>...> end) : m_end(end) { }

//             sentinel() = default;

//             constexpr sentinel(sentinel<!Const> i)
//                 requires Const && (std::convertible_to<std::ranges::sentinel_t<Vs>, std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>> && ...) : m_end(std::move(i.m_end)) { }
            

//             template <bool OtherConst>
//                 requires (std::sentinel_for<std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>,
//                                             std::ranges::iterator_t<detail::maybe_const_t<OtherConst, Vs>>> && ...)
//             friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
//             {
//                 return [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
//                     return (static_cast<bool>(std::get<Idx>(x.m_current) == std::get<Idx>(y.m_end)) || ...);
//                 }(std::index_sequence_for<Vs...>());
//             }

//             template <bool OtherConst>
//                 requires (std::sized_sentinel_for<std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>,
//                                 std::ranges::sentinel_t<detail::maybe_const_t<OtherConst, Vs>>> && ...)
//             friend constexpr std::common_type_t<std::ranges::range_difference_t<detail::maybe_const_t<OtherConst, Vs>>...> operator-(const iterator<OtherConst>& x, const sentinel& y)
//             {
//                 using diff = std::common_type_t<std::ranges::range_difference_t<detail::maybe_const_t<OtherConst, Vs>>...>;
//                 return [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
//                     diff D[] = { diff(std::get<Idx>(x.m_current) - std::get<Idx>(y.m_end))... };
//                     return std::ranges::min(D);
//                 }(std::index_sequence_for<Vs...>());
//             }   

//             template <bool OtherConst>
//                 requires (std::sized_sentinel_for<std::ranges::sentinel_t<detail::maybe_const_t<Const, Vs>>,
//                                 std::ranges::sentinel_t<detail::maybe_const_t<OtherConst, Vs>>> && ...)
//             friend constexpr std::common_type_t<std::ranges::range_difference_t<detail::maybe_const_t<OtherConst, Vs>>...> operator-(const sentinel& y, const iterator<OtherConst>& x)
//             {
//                 return -(x - y);
//             }  

//         };

//     public:
//         zip_view() = default;
        
//         constexpr explicit zip_view(Vs... vs) : m_views(std::move(vs)...) { }
        
//         constexpr auto begin() requires (!(detail::simple_view<Vs> && ...))
//         {
//             return iterator<false>(detail::tuple_transform(std::ranges::begin, m_views));
//         }   

//         constexpr auto begin() const requires (std::ranges::range<const Vs> && ...) 
//         {
//             return iterator<true>(detail::tuple_transform(std::ranges::begin, m_views));
//         }

//         constexpr auto end() requires (!(detail::simple_view<Vs> && ...))
//         {
//             // random-access-range may not common range
//             // std::ranges::end for some random-access-range may return sentinel
//             if constexpr (!zip_is_common<Vs...>)
//                 return sentinel<false>(detail::tuple_transform(std::ranges::end, m_views));
//             else if constexpr ((std::ranges::random_access_range<Vs> && ...))
//                 return begin() + std::iter_difference_t<iterator<false>>(size());
//             else
//                 return iterator<false>(detail::tuple_transform(std::ranges::end, m_views));
//         }   

//         constexpr auto end() const requires (std::ranges::range<const Vs> && ...)
//         {
//             if constexpr (!zip_is_common<const Vs...>)
//                 return sentinel<true>(detail::tuple_transform(std::ranges::end, m_views));
//             else if constexpr ((std::ranges::random_access_range<const Vs> && ...))
//                 return begin() + std::iter_difference_t<iterator<true>>(size());
//             else
//                 return iterator<true>(detail::tuple_transform(std::ranges::end, m_views));
//         }

//         constexpr auto size() requires (std::ranges::sized_range<Vs> && ...)
//         {
//             return std::apply([](auto... sizes) {
//                 // FIXME:
//                 // using CT = std::make_unsigned_t<std::common_type_t<decltype(sizes)...>>;
//                 using CT = std::size_t;
//                 // std::array sizes = ;
//                 return std::ranges::min({ CT{sizes}... });
//             }, detail::tuple_transform(std::ranges::size, m_views));
//         }

//         constexpr auto size() const requires (std::ranges::sized_range<const Vs> && ...)
//         {
//             return std::apply([](auto... sizes) {
//                 // FIXME:
//                 using CT = std::make_unsigned_t<std::common_type_t<decltype(sizes)...>>;
//                 return std::ranges::min({ CT{sizes}...} );
//             }, detail::tuple_transform(std::ranges::size, m_views));
//         }

//     };

//     template <typename... Rs>
//     zip_view(Rs&&...) -> zip_view<std::views::all_t<Rs>...>;

//     template <typename... Rs>
//     concept can_zippable = requires 
//     {
//         zip_view{ std::declval<Rs>()... };
//     };

//     struct zip_adaptor
//     {
//         template <std::ranges::viewable_range... Rs>
//             requires can_zippable<Rs...>
//         constexpr auto operator()(Rs&&... rs) const
//         {
//             return zip_view{ (Rs&&)rs... };
//         }
//     };

//     inline constexpr zip_adaptor zip{};

//     struct zip_with_adaptor : range_adaptor<zip_with_adaptor>
//     {
//         using range_adaptor<zip_with_adaptor>::operator();

//         template <std::ranges::viewable_range R1, std::ranges::viewable_range R2>
//             requires can_zippable<R1, R2>
//         constexpr auto operator()(R1&& r1, R2&& r2) const
//         {
//             return zip_view{ (R1&&)r1, (R2&&)r2 };
//         }

//     };

//     inline constexpr zip_with_adaptor zip_with{};

// }

// namespace std::ranges
// {
//     template <typename... Views>
//     inline constexpr bool enable_borrowed_range<::leviathan::ranges::zip_view<Views...>> = 
//         (enable_borrowed_range<Views> && ...);
// }
