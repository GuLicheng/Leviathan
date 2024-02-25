// #pragma once

// #include "common.hpp"

// // ranges.chunk_view
// namespace leviathan::ranges
// {
//     // For num = 3, denom = 2, div_ceil = 2
//     // For num = 3, denom = 3, div_ceil = 1
//     template <typename I>
//     constexpr I div_ceil(I num, I denom)
//     {
//         I r = num / denom;
//         if (num % denom) ++r;
//         return r;
//     }

//     template <std::ranges::view V>
//         requires std::ranges::input_range<V>
//     class chunk_view : public std::ranges::view_interface<chunk_view<V>>
//     {
//         // For V = [1, 2, 3, 4, 5], n = 2
//         //         [1, 2]         
//         //               [3, 4]   
//         //                     [5]

//         V m_base = V();
//         std::ranges::range_difference_t<V> m_n = 0;
//         std::ranges::range_difference_t<V> m_remainder = 0;

//         using non_propagating_cache = std::optional<std::ranges::iterator_t<V>>;

//         non_propagating_cache m_current;

//         struct outer_iterator;
//         struct inner_iterator;

//     public:

//         chunk_view() requires std::default_initializable<V> = default;

//         constexpr explicit chunk_view(V base, std::ranges::range_difference_t<V> n)
//             : m_base(std::move(base)), m_n(n)
//         {
//             assert(n > 0);
//         }

//         constexpr V base() const& requires std::copy_constructible<V> { return m_base; }
//         constexpr V base() && { return std::move(m_base); }

//         constexpr outer_iterator begin()
//         {
//             m_current = std::ranges::begin(m_base);
//             m_remainder = m_n;
//             return outer_iterator(*this);
//         }
        
//         constexpr std::default_sentinel_t end() noexcept
//         {
//             return std::default_sentinel;
//         }

//         constexpr auto size() requires std::ranges::sized_range<V>
//         {
//             // FIXME: 
//             using T = std::make_unsigned_t<decltype(div_ceil(std::ranges::distance(m_base), m_n))>;
//             return T(div_ceil(std::ranges::distance(m_base), m_n));
//         }

//         constexpr auto size() const requires std::ranges::sized_range<const V>
//         {
//             // FIXME: 
//             using T = std::make_unsigned_t<decltype(div_ceil(std::ranges::distance(m_base), m_n))>;
//             return T(div_ceil(std::ranges::distance(m_base), m_n));
//         }

//     private:

//         struct outer_iterator
//         {
//             chunk_view* m_parent;
//             constexpr explicit outer_iterator(chunk_view& parent) 
//                 : m_parent(std::addressof(parent)) { }
        
//             using iterator_concept = std::input_iterator_tag;
//             using difference_type = std::ranges::range_difference_t<V>;

//             struct value_type : std::ranges::view_interface<value_type>
//             {
//                 chunk_view* m_p;

//                 constexpr explicit value_type(chunk_view& p) : m_p(std::addressof(p)) { }

//             public:
//                 constexpr inner_iterator begin() const noexcept
//                 {
//                     return inner_iterator(*m_p);
//                 }

//                 constexpr std::default_sentinel_t end() const noexcept
//                 {
//                     return std::default_sentinel;
//                 }

//                 constexpr auto size() const 
//                     requires std::sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
//                 {
//                     return std::ranges::min(m_parent->m_remainder, std::ranges::end(m_parent->m_base) - *m_parent->m_current);
//                 }
                
//             };

//             outer_iterator(outer_iterator&&) = default;
//             outer_iterator& operator=(outer_iterator&&) = default;

//             constexpr value_type operator*() const
//             {
//                 assert(*this != std::default_sentinel);
//                 return value_type(*m_parent);
//             }

//             constexpr outer_iterator& operator++()
//             {
//                 assert(*this != std::default_sentinel);
//                 std::ranges::advance(m_parent->m_current, m_parent->m_remainder, std::ranges::end(m_parent->m_base));
//                 return *this;
//             }

//             constexpr void operator++(int)
//             {
//                 (void)(++*this);
//             }

//             friend constexpr bool operator==(const outer_iterator& x, std::default_sentinel_t)
//             {
//                 return *x.m_parent->m_current == std::ranges::end(x.m_parent->m_base) 
//                     && x.m_parent->m_remainder != 0;
//             }

//             friend constexpr difference_type operator-(std::default_sentinel_t, const outer_iterator& x)
//                 requires std::sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
//             {
//                 const auto dist = std::ranges::end(x.m_parent->m_base) - *x.m_parent->m_current;
//                 if (dist < x.m_parent->m_remainder)
//                     return dist == 0 ? 0 : 1;
//                 return div_ceil(dist - x.m_parent->m_remainder, x.m_parent->m_n) + 1;
//             }

//             friend constexpr difference_type operator-(const outer_iterator& x, std::default_sentinel_t y)
//                 requires std::sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
//             {
//                 return -(y - x);
//             }
            
//         };

//         struct inner_iterator
//         {
//             chunk_view* m_p;
//             constexpr explicit inner_iterator(chunk_view& p) noexcept : m_p(std::addressof(p)) { }

//             using iterator_concept = std::input_iterator_tag;
//             using difference_type = std::ranges::range_difference_t<V>;
//             using value_type = std::ranges::range_value_t<V>;

//             inner_iterator(inner_iterator&&) = default;
//             inner_iterator& operator=(inner_iterator&&) = default;

//             constexpr const std::ranges::iterator_t<V> base() const&
//             {
//                 return *m_p->m_current;
//             }

//             constexpr std::ranges::range_reference_t<V> operator*() const
//             {
//                 assert(*this != std::default_sentinel);
//                 return **m_p->m_current;
//             }

//             constexpr inner_iterator& operator++()
//             {
//                 ++*m_p->m_current;
//                 if (*m_p->m_current == std::ranges::end(m_p->m_base))
//                     m_p->m_remainder = 0;
//                 else
//                     --m_p->m_remainder;
//                 return *this;
//             }

//             constexpr void operator++(int)
//             {
//                 (void)(++*this);
//             }

//             friend constexpr bool operator==(const inner_iterator& x, std::default_sentinel_t)
//             {
//                 return x.m_p->m_remainder == 0;
//             }

//             friend constexpr difference_type operator-(std::default_sentinel_t, const inner_iterator& x)
//                 requires std::sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
//             {
//                 return std::ranges::min(x.m_p->m_remainder, std::ranges::end(x.m_p->m_base) - *x.m_p->m_current);
//             }

//             friend constexpr difference_type operator-(const inner_iterator& x, std::default_sentinel_t y)
//                 requires std::sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
//             {
//                 return -(y - x);
//             } 

//         };

//     };

//     template <typename R>
//     chunk_view(R&& r, std::ranges::range_difference_t<R>) -> chunk_view<std::views::all_t<R>>;

//     template <std::ranges::view V>
//         requires std::ranges::forward_range<V>
//     class chunk_view<V> : public std::ranges::view_interface<chunk_view<V>>
//     {
//         V m_base = V();
//         std::ranges::range_difference_t<V> m_n = 0;

//         template <bool> struct iterator;

//     public:

//         chunk_view() requires std::default_initializable<V> = default;
        
//         constexpr explicit chunk_view(V base, std::ranges::range_difference_t<V> n)
//             : m_base(std::move(base)), m_n(n) 
//         { assert(n > 0); }

//         constexpr V base() const& requires std::copy_constructible<V> { return m_base; }
//         constexpr V base() && { return std::move(m_base); }

//         constexpr auto begin() requires (!detail::simple_view<V>) 
//         {
//             return iterator<false>(this, std::ranges::begin(m_base));
//         }

//         constexpr auto begin() const requires std::ranges::forward_range<const V>
//         {
//             return iterator<true>(this, std::ranges::begin(m_base));
//         }

//         constexpr auto end() requires (!detail::simple_view<V>)
//         {
//             if constexpr (std::ranges::common_range<V> && std::ranges::sized_range<V>) 
//             {
//                 // for V = [1, 2, 3, 4, 5], n = 2
//                 // end is (2 - 5 % 2) % 2 = 1
//                 auto missing = (m_n - std::ranges::distance(m_base) % m_n) % m_n;
//                 return iterator<false>(this, std::ranges::end(m_base), missing);
//             }
//             else if constexpr (std::ranges::common_range<V> && !std::ranges::bidirectional_range<V>) 
//             {
//                 return iterator<false>(this, std::ranges::end(m_base));
//             }
//             else
//             {
//                 return std::default_sentinel;
//             }
//         }

//         constexpr auto end() const requires std::ranges::forward_range<const V>
//         {
//             if constexpr (std::ranges::common_range<const V> && std::ranges::sized_range<const V>)
//             {
//                 auto missing = (m_n - std::ranges::distance(m_base) % m_n) % m_n;
//                 return iterator<true>(this, std::ranges::end(m_base), missing);
//             }
//             else if constexpr (std::ranges::common_range<const V> && !std::ranges::bidirectional_range<const V>) 
//             {
//                 return iterator<true>(this, std::ranges::end(m_base));
//             }
//             else
//             {
//                 return std::default_sentinel;
//             }
//         }

//         constexpr auto size() requires std::ranges::sized_range<V>
//         {
//             using T = std::make_unsigned_t<decltype(div_ceil(std::ranges::distance(m_base), m_n))>;
//             return T(div_ceil(std::ranges::distance(m_base), m_n));
//         }

//         constexpr auto size() const requires std::ranges::sized_range<const V>
//         {
//             using T = std::make_unsigned_t<decltype(div_ceil(std::ranges::distance(m_base), m_n))>;
//             return T(div_ceil(std::ranges::distance(m_base), m_n));
//         }

//     private:
//         template <bool Const> 
//         struct iterator
//         {
//             using parent = detail::maybe_const_t<Const, chunk_view>;
//             using Base = detail::maybe_const_t<Const, V>;

//             std::ranges::iterator_t<Base> m_current = std::ranges::iterator_t<Base>();
//             std::ranges::sentinel_t<Base> m_end = std::ranges::sentinel_t<Base>();

//             std::ranges::range_difference_t<Base> m_n = 0;
//             std::ranges::range_difference_t<Base> m_missing = 0;

//             constexpr iterator(parent* p, std::ranges::iterator_t<Base> current, std::ranges::range_difference_t<Base> missing = 0) : m_current(std::move(current)), m_end(std::ranges::end(p->m_base)), m_n(p->m_n), m_missing(missing) { }

//             using iterator_category = std::input_iterator_tag;
//             using iterator_concept = decltype(detail::simple_iterator_concept<Base>());
//             using value_type = decltype(std::views::take(std::ranges::subrange(m_current, m_end), m_n));
//             using difference_type = std::ranges::range_difference_t<Base>;

//             iterator() = default;

//             constexpr iterator(iterator<!Const> i)
//                 requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<Base>>
//                                && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<Base>>
//                 : m_current(std::move(i.m_current)), m_end(std::move(i.m_end)), m_n(i.m_n), m_missing(i.m_missing) { } 

//             constexpr std::ranges::iterator_t<Base> base() const { return m_current; }

//             constexpr value_type operator*() const
//             {
//                 assert(m_current != m_end);
//                 return std::views::take(std::ranges::subrange(m_current, m_end), m_n);
//             }

//             constexpr iterator& operator++()
//             {
//                 assert(m_current != m_end);
//                 m_missing = std::ranges::advance(m_current, m_n, m_end);
//                 return *this;
//             }

//             constexpr iterator operator++(int)
//             {
//                 auto temp = *this;
//                 ++*this;
//                 return temp;
//             }

//             constexpr iterator& operator--() requires std::ranges::bidirectional_range<Base>
//             {
//                 std::ranges::advance(m_current, m_missing - m_n);
//                 m_missing = 0;
//                 return *this;
//             }

//             constexpr iterator operator--(int) requires std::ranges::bidirectional_range<Base>
//             {
//                 auto temp = *this;
//                 --*this;
//                 return temp;
//             }

//             constexpr iterator& operator+=(difference_type x)
//                 requires std::ranges::random_access_range<Base>
//             {
//                 if (x > 0)
//                 {
//                     assert(std::ranges::distance(m_current, m_end) > m_n * (x - 1));
//                     m_missing = std::ranges::advance(m_current, m_n * x, m_end);
//                 }
//                 else if (x < 0)
//                 {
//                     std::ranges::advance(m_current, m_n * x + m_missing);
//                     m_missing = 0;
//                 }
//                 return *this;
//             }

//             constexpr iterator& operator-=(difference_type x)
//                 requires std::ranges::random_access_range<Base>
//             {
//                 return *this += -x;
//             }
        
//             constexpr value_type operator[](difference_type x) const
//                 requires std::ranges::random_access_range<Base>
//             {
//                 return *(*this + x);
//             }

//             friend constexpr bool operator==(const iterator& x, const iterator& y)
//             {
//                 return x.m_current == y.m_current;
//             }

//             friend constexpr bool operator==(const iterator& x, std::default_sentinel_t)
//             {
//                 return x.m_current == x.m_end;
//             }
        
//             friend constexpr bool operator<(const iterator& x, const iterator& y)
//                 requires std::ranges::random_access_range<Base>
//             {
//                 return x.m_current < y.m_current;
//             }

//             friend constexpr bool operator>(const iterator& x, const iterator& y)
//                 requires std::ranges::random_access_range<Base>
//             {
//                 return y < x;
//             }

//             friend constexpr bool operator<=(const iterator& x, const iterator& y)
//                 requires std::ranges::random_access_range<Base>
//             {
//                 return !(y < x);
//             }

//             friend constexpr bool operator>=(const iterator& x, const iterator& y)
//                 requires std::ranges::random_access_range<Base>
//             {
//                 return !(x < y);
//             }

//             friend constexpr bool operator<=>(const iterator& x, const iterator& y)
//                 requires std::three_way_comparable<std::ranges::iterator_t<Base>>
//             {
//                 return x.m_current <=> y.m_current;
//             }

//             friend constexpr iterator operator+(const iterator& i, difference_type n)
//                 requires std::ranges::random_access_range<Base>
//             {
//                 auto r = i;
//                 r += n;
//                 return r;
//             }

//             friend constexpr iterator operator+(difference_type n, const iterator& i)
//                 requires std::ranges::random_access_range<Base>
//             {
//                 auto r = i;
//                 r += n;
//                 return r;
//             }

//             friend constexpr iterator operator-(const iterator& i, difference_type n)
//                 requires std::ranges::random_access_range<Base>
//             {
//                 auto r = i;
//                 r -= n;
//                 return r;
//             }

//             friend constexpr difference_type operator-(const iterator& x, const iterator& y)
//                 requires std::sized_sentinel_for<std::ranges::iterator_t<Base>, std::ranges::iterator_t<Base>>
//             {
//                 return (x.m_current - y.m_current + x.m_missing - y.m_missing) / x.m_n;
//             }

//             friend constexpr difference_type operator-(const iterator& x, std::default_sentinel_t y)
//                 requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>>
//             {
//                 return -(y - x);
//             }

//             friend constexpr difference_type operator-(std::default_sentinel_t y, const iterator& x)
//                 requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>>
//             {
//                 return div_ceil(x.m_end - x.m_current, x.m_n);
//             }

//         };

//     };

//     template <typename R, typename N>
//     concept can_chunk = requires
//     {
//         chunk_view(std::declval<R>(), std::declval<N>());
//     };

//     struct chunk_adaptor : range_adaptor<chunk_adaptor>
//     {
//         using range_adaptor<chunk_adaptor>::operator();

//         template <std::ranges::viewable_range R, std::integral N>
//             requires can_chunk<R, N>
//         constexpr auto operator()(R&& r, N n) const
//         {
//             return chunk_view((R&&)r, n);
//         }
//     };

//     inline constexpr chunk_adaptor chunk{};

// }

// namespace std::ranges
// {
//     template <typename V>
//     inline constexpr bool enable_borrowed_range<::leviathan::ranges::chunk_view<V>> = 
//         enable_borrowed_range<V> && std::ranges::forward_range<V>;
// }
