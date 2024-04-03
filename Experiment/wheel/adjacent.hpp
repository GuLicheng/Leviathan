// #pragma once

// #include "common.hpp"

// // range.adjacent_view
// namespace leviathan::ranges
// {
//     template <std::size_t N, typename T1, typename... Ts>
//     struct adjacent_repeat_value_type_impl : adjacent_repeat_value_type_impl<N - 1, T1, T1, Ts...> { };

//     template <typename T1, typename... Ts>
//     struct adjacent_repeat_value_type_impl<0, T1, Ts...> 
//         : std::type_identity<detail::tuple_or_pair<T1, Ts...>> { };

//     // for N = 2, the loop will start from 2 and end by 0, which contains three elements(0, 1, 2)
//     template <typename T, std::size_t N>
//     struct adjacent_repeat_value_type : adjacent_repeat_value_type_impl<N - 1, T> { };

//     template <std::ranges::forward_range V, std::size_t N>
//         requires std::ranges::view<V> && (N > 0)
//     class adjacent_view : public std::ranges::view_interface<adjacent_view<V, N>>
//     {
//         V m_base = V();

//         template <bool> struct iterator;
//         template <bool> struct sentinel;
//         struct as_sentinel { };
    
//     public:
//         adjacent_view() requires std::default_initializable<V>  = default;

//         constexpr explicit adjacent_view(V base) : m_base(std::move(base)) { }

//         constexpr auto begin() requires (!detail::simple_view<V>)
//         {
//             return iterator<false>(std::ranges::begin(m_base), std::ranges::end(m_base));
//         }

//         constexpr auto begin() const requires std::ranges::range<const V>
//         {
//             return iterator<true>(std::ranges::begin(m_base), std::ranges::end(m_base));
//         }        
    
//         constexpr auto end() requires (!detail::simple_view<V>)
//         {
//             if constexpr (std::ranges::common_range<V>)
//                 return iterator<false>(as_sentinel{}, std::ranges::begin(m_base), std::ranges::end(m_base));
//             else
//                 return sentinel<false>(std::ranges::end(m_base));
//         }

//         constexpr auto end() const requires std::ranges::range<const V>
//         {
//             if constexpr (std::ranges::common_range<V>)
//                 return iterator<true>(as_sentinel{}, std::ranges::begin(m_base), std::ranges::end(m_base));
//             else
//                 return sentinel<true>(std::ranges::end(m_base));
//         }

//         constexpr auto size() requires std::ranges::sized_range<V>
//         {
//             using ST = decltype(std::ranges::size(m_base));
//             using CT = std::common_type_t<ST, std::size_t>;
//             auto sz = static_cast<CT>(std::ranges::size(m_base));
//             sz -= std::min<CT>(sz, N - 1);
//             return static_cast<ST>(sz);
//         }

//         constexpr auto size() const requires std::ranges::sized_range<const V>
//         {
//             using ST = decltype(std::ranges::size(m_base));
//             using CT = std::common_type_t<ST, std::size_t>;
//             auto sz = static_cast<CT>(std::ranges::size(m_base));
//             sz -= std::min<CT>(sz, N - 1);
//             return static_cast<ST>(sz);
//         }

//     private:

//         template <bool Const> 
//         struct iterator
//         {
//             using base = detail::maybe_const_t<Const, V>;
//             std::array<std::ranges::iterator_t<base>, N> m_current = std::array<std::ranges::iterator_t<base>, N>();

//             constexpr iterator(std::ranges::iterator_t<base> first, std::ranges::sentinel_t<base> last)
//             {
//                 m_current[0] = first;
//                 for (std::size_t i = 1; i < N; ++i)
//                     m_current[i] = std::ranges::next(m_current[i - 1], 1, last);
//             }
            
//             constexpr iterator(as_sentinel, std::ranges::iterator_t<base> first, std::ranges::sentinel_t<base> last)
//             {
//                 if constexpr (!std::ranges::bidirectional_range<base>)
//                 {
//                     std::ranges::fill(m_current, last);
//                 }
//                 else
//                 {
//                     m_current[N - 1] = last;
//                     // if N == 1, i = -1 -> end of loop
//                     // if N == 2, i = 0 -> one iteration
//                     for (auto i = N - 2; i != std::size_t(-1); --i)
//                         m_current[i] = std::ranges::prev(m_current[i + 1], 1, first);
//                 }
//             }

//             using iter_value_type = std::ranges::range_value_t<base>;

//             using iterator_category = std::input_iterator_tag;
//             using iterator_concept = decltype(detail::simple_iterator_concept<base>());
//             using value_type = typename adjacent_repeat_value_type<iter_value_type, N>::type;
//             using difference_type = std::ranges::range_difference_t<base>;
            
//             iterator() = default;

//             constexpr iterator(iterator<!Const> i) 
//                 requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<base>>
//             {
//                 // for (std::size_t i = 0; i < N; ++i)
//                     // m_current[i] = std::move(i.m_current[i]);
//                 std::ranges::move(i.m_current, std::ranges::begin(m_current));
//             }

//             constexpr auto operator*() const
//             {
//                 return detail::tuple_transform([](auto& i) -> decltype(auto) {
//                     return *i;
//                 }, m_current);
//             }

//             constexpr iterator& operator++()
//             {
//                 std::shift_left(m_current.begin(), m_current.end(), 1);
//                 m_current.back() = std::ranges::next(m_current[N - 2], 1);
//                 return *this;                
//             }

//             constexpr iterator operator++(int) 
//             {
//                 auto temp = *this;
//                 ++*this;
//                 return temp;
//             }

//             constexpr iterator& operator--() requires std::ranges::bidirectional_range<base>
//             {
//                 std::shift_right(m_current.begin(), m_current.end(), 1);
//                 m_current.front() = std::ranges::prev(m_current[1], 1);
//                 return *this;                
//             }

//             constexpr iterator operator--(int) 
//             {
//                 auto temp = *this;
//                 --*this;
//                 return temp;
//             }

//             constexpr iterator& operator+=(difference_type x)
//                 requires std::ranges::random_access_range<base>
//             {
//                 std::ranges::for_each(m_current, [&](auto& i) {
//                     i += x;
//                 });
//                 return *this;
//             }

//             constexpr iterator& operator-=(difference_type x)
//                 requires std::ranges::random_access_range<base>
//             {
//                 std::ranges::for_each(m_current, [&](auto& i) {
//                     i -= x;
//                 });
//                 return *this;
//             }

//             constexpr auto operator[](difference_type n) const
//             {
//                 return detail::tuple_transform([&](auto& i) -> decltype(auto) {
//                     return i[n];
//                 }, m_current);
//             }

//             friend constexpr bool operator==(const iterator& x, const iterator& y)
//             {
//                 return x.m_current.back() == y.m_current.back();
//             }

//             friend constexpr bool operator<(const iterator& x, const iterator& y)
//                 requires std::ranges::random_access_range<base>
//             {
//                 return x.m_current.back() < y.m_current.back();
//             }

//             friend constexpr bool operator<=(const iterator& x, const iterator& y)
//                 requires std::ranges::random_access_range<base>
//             {
//                 return !(y < x);
//             }

//             friend constexpr bool operator>(const iterator& x, const iterator& y)
//                 requires std::ranges::random_access_range<base>
//             {
//                 return y < x; 
//             }
            
//             friend constexpr bool operator>=(const iterator& x, const iterator& y)
//                 requires std::ranges::random_access_range<base>
//             {
//                 return !(x < y);
//             }

//             friend constexpr bool operator<=>(const iterator& x, const iterator& y)
//                 requires std::ranges::random_access_range<base> && 
//                          std::three_way_comparable<std::ranges::iterator_t<base>>
//             {
//                 return x.m_current.back() <=> y.m_current.back();
//             }

//             friend constexpr iterator operator+(const iterator& i, difference_type n)
//                 requires std::ranges::random_access_range<base>
//             {
//                 auto r = i;
//                 r += n;
//                 return r;
//             }

//             friend constexpr iterator operator+(difference_type n, const iterator& i)
//                 requires std::ranges::random_access_range<base>
//             {
//                 auto r = i;
//                 r += n;
//                 return r;
//             }

//             friend constexpr iterator operator-(const iterator& i, difference_type n)
//                 requires std::ranges::random_access_range<base>
//             {
//                 auto r = i;
//                 r -= n;
//                 return r;
//             }

//             friend constexpr iterator operator-(const iterator& x, const iterator& y)
//                 requires std::sized_sentinel_for<std::ranges::iterator_t<base>, std::ranges::iterator_t<base>>
//             {
//                 return x.m_current.back() - y.m_current.back();
//             }

//             friend constexpr auto iter_move(const iterator& i) noexcept(noexcept(std::ranges::iter_move(std::declval<const std::ranges::iterator_t<base>&>())) && std::is_nothrow_move_constructible_v<std::ranges::range_rvalue_reference_t<base>>)
//             {
//                 return detail::tuple_transform(std::ranges::iter_move, i.m_current);
//             }

//             friend constexpr auto iter_swap(const iterator& l, const iterator& r) noexcept(noexcept(std::ranges::iter_swap(std::declval<std::ranges::iterator_t<base>>(), std::ranges::iterator_t<base>())))
//                 requires std::indirectly_swappable<std::ranges::iterator_t<base>>
//             {
//                 for (std::size_t i = 0; i < N; ++i)
//                     std::ranges::iter_swap(l.m_current[i], r.m_current[i]);
//             }

//         };

//         template <bool Const> 
//         struct sentinel
//         {
//             using base = detail::maybe_const_t<Const, V>;
//             std::ranges::sentinel_t<base> m_end = std::ranges::sentinel_t<base>();
//             constexpr explicit sentinel(std::ranges::sentinel_t<base> end) : m_end(std::move(end)) { }

//             sentinel() = default;

//             template <bool OtherConst>
//                 requires std::sentinel_for<std::ranges::sentinel_t<base>, std::ranges::iterator_t<detail::maybe_const_t<OtherConst, V>>>
//             friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
//             {
//                 return x.m_current.back() == y.m_end;
//             }

//             template <bool OtherConst>
//                 requires std::sized_sentinel_for<std::ranges::sentinel_t<base>, std::ranges::iterator_t<detail::maybe_const_t<OtherConst, V>>>
//             friend constexpr std::ranges::range_difference_t<detail::maybe_const_t<OtherConst, V>> operator-(const iterator<OtherConst>& x, const sentinel& y)
//             {
//                 return x.m_current.back() - y.m_end;
//             }

//             template <bool OtherConst>
//                 requires std::sized_sentinel_for<std::ranges::sentinel_t<base>, std::ranges::iterator_t<detail::maybe_const_t<OtherConst, V>>>
//             friend constexpr std::ranges::range_difference_t<detail::maybe_const_t<OtherConst, V>> operator-(const iterator<OtherConst>& x, const sentinel& y)
//             {
//                 return y.m_end - x.m_current.back();
//             }
//         };

//     };

//     // template <typename R, std::size_t N>
//     // adjacent_view(R&&) -> adjacent_view<std::views::all_t<R>, N>;

//     template <std::size_t N>
//     struct adjacent_adaptor : range_adaptor_closure
//     {
//         template <std::ranges::viewable_range R>
//         constexpr auto operator()(R&& r) const
//         {
//             return adjacent_view<std::views::all_t<R>, N>((R&&)r);
//         }
//     };

//     template <std::size_t N>
//     inline constexpr auto adjacent = adjacent_adaptor<N>();

// }

// namespace std::ranges
// {
//     template <typename V, std::size_t N>
//     inline constexpr bool enable_borrowed_range<::leviathan::ranges::adjacent_view<V, N>> = 
//         enable_borrowed_range<V>;
// }
