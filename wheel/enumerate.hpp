// #pragma once

// #include "common.hpp"

// // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2164r5.pdf
// namespace leviathan::ranges
// {
//     template <typename Index, typename Value>
//     struct enumerate_result
//     {
//         Index index;
//         Value value;
//     };

//     template <std::ranges::input_range V>
//     requires std::ranges::view<V>
//     class enumerate_view : public std::ranges::view_interface<enumerate_view<V>>
//     {
//     private:
//         V m_base = V();
//         template <bool Const> struct iterator;
//         template <bool Const> struct sentinel;

//     public:
//         constexpr enumerate_view() = default;
//         constexpr enumerate_view(V base) : m_base{ std::move(base) } { }

//         constexpr auto begin() requires (!detail::simple_view<V>)
//         { return iterator<false>(std::ranges::begin(m_base), 0); }

//         constexpr auto begin() const requires detail::simple_view<V>
//         { return iterator<true>(std::ranges::begin(m_base), 0); }

//         constexpr auto end()
//         { return sentinel<false>{std::ranges::end(m_base)}; }

//         constexpr auto end()
//         requires std::ranges::common_range<V> && std::ranges::sized_range<V>
//         {
//             using index_type = typename iterator<false>::index_type;
//             return iterator<false>{std::ranges::end(m_base), static_cast<index_type>(size()) }; 
//         }

//         constexpr auto end() const
//         requires std::ranges::range<const V>
//         { return sentinel<true>(std::ranges::end(m_base)); }

//         constexpr auto end() const
//         requires std::ranges::common_range<const V> && std::ranges::sized_range<V>
//         { return iterator<true>(std::ranges::end(m_base), static_cast<std::ranges::range_difference_t<V>>(size())); }

//         constexpr auto size()
//         requires std::ranges::sized_range<V>
//         { return std::ranges::size(m_base); }

//         constexpr auto size() const
//         requires std::ranges::sized_range<const V>
//         { return std::ranges::size(m_base); }

//         constexpr V base() const& requires std::copy_constructible<V> { return m_base; }
//         constexpr V base() && { return std::move(m_base); }

//     };

//     template <typename R>
//     enumerate_view(R&&) -> enumerate_view<std::views::all_t<R>>;

//     template <std::ranges::input_range V>
//     requires std::ranges::view<V>
//     template <bool Const>
//     struct enumerate_view<V>::iterator : detail::has_typedef_name_of_iterator_category<detail::has_iterator_category<std::ranges::iterator_t<std::conditional_t<Const, const V, V>>>, detail::iter_category_t<std::ranges::iterator_t<std::conditional_t<Const, const V, V>>>>
//     {
//         using base_type = std::conditional_t<Const, const V, V>;
//         using index_type = std::conditional_t<std::ranges::sized_range<base_type>, std::ranges::range_size_t<base_type>, std::make_unsigned_t<std::ranges::range_difference_t<base_type>>>;

//         std::ranges::iterator_t<base_type> m_current = std::ranges::iterator_t<base_type>();
//         index_type m_pos = 0;

//     public:

//         // using iterator_category = typename std::iterator_traits<std::ranges::iterator_t<base_type>>::iterator_category;
//         // using reference = enumerate_result<index_type, std::ranges::range_reference_t<base_type>>;
//         using reference = std::pair<index_type, std::ranges::range_reference_t<base_type>>;
//         using value_type = std::tuple<index_type, std::ranges::range_value_t<base_type>>;
//         using difference_type = std::ranges::range_difference_t<base_type>;

//         iterator() = default;

//         constexpr explicit iterator(std::ranges::iterator_t<base_type> current, index_type pos = 0) : m_current{ std::move(current) }, m_pos{ pos } { }

//         constexpr iterator(iterator<!Const> i)
//         requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<base_type>> :
//         m_current{ std::move(i.m_current) }, m_pos{ i.m_pos } { }

//         constexpr std::ranges::iterator_t<base_type> base() const&
//         requires std::copyable<std::ranges::iterator_t<base_type>>
//         { return m_current; }

//         constexpr std::ranges::iterator_t<base_type> base() &&
//         { return std::move(m_current); }

//         constexpr decltype(auto) operator*() const
//         { return reference{ m_pos, *m_current }; }

//         constexpr iterator& operator++()
//         {
//             ++m_pos;
//             ++m_current;
//             return *this;
//         }

//         constexpr auto operator++(int)
//         {
//             if constexpr (std::ranges::forward_range<base_type>)
//             {
//                 auto temp = *this;
//                 ++*this;
//                 return temp;
//             }
//             else
//             {
//                 (void)(this->operator++());
//             }
//         }

//         constexpr iterator& operator--() requires std::ranges::bidirectional_range<base_type>
//         {
//             --m_pos;
//             --m_current;
//             return *this;
//         }

//         constexpr iterator operator--(int) requires std::ranges::bidirectional_range<base_type>
//         {
//             auto temp = *this;
//             --*this;
//             return temp;
//         }

//         constexpr iterator& operator+=(difference_type n) 
//         requires std::ranges::random_access_range<base_type>
//         {
//             m_current += n;
//             m_pos += n;
//             return *this;
//         }
        
//         constexpr iterator& operator-=(difference_type n) 
//         requires std::ranges::random_access_range<base_type>
//         {
//             m_current -= n;
//             m_pos -= n;
//             return *this; 
//         }

//         constexpr decltype(auto) operator[](difference_type n) const
//         requires std::ranges::random_access_range<base_type>
//         { return reference{ static_cast<difference_type>(m_pos + n), *(m_current + n) }; }

//         friend constexpr bool operator==(const iterator& x, const iterator& y)
//         requires std::equality_comparable<std::ranges::iterator_t<base_type>>
//         { return x.m_current == y.m_current; }

//         friend constexpr bool operator<(const iterator& x, const iterator& y)
//         requires std::ranges::random_access_range<base_type>
//         { return x.m_current < y.m_current; }

//         friend constexpr bool operator>(const iterator& x, const iterator& y)
//         requires std::ranges::random_access_range<base_type>
//         { return y < x; }

//         friend constexpr bool operator<=(const iterator& x, const iterator& y)
//         requires std::ranges::random_access_range<base_type>
//         { return !(y < x); }

//         friend constexpr bool operator>=(const iterator& x, const iterator& y)
//         requires std::ranges::random_access_range<base_type>
//         { return !(x < y); }
        
//         friend constexpr auto operator<=>(const iterator& x, const iterator& y)
//         requires std::ranges::random_access_range<base_type> && std::three_way_comparable<std::ranges::iterator_t<base_type>>
//         { return x.m_current <=> y.m_current; }

//         friend constexpr iterator operator+(const iterator& x, difference_type y)
//         requires std::ranges::random_access_range<base_type>
//         { return iterator{x} += y; }

//         friend constexpr iterator operator+(difference_type x, const iterator& y)
//         requires std::ranges::random_access_range<base_type>
//         { return y + x; }

//         friend constexpr iterator operator-(const iterator& x, difference_type y)
//         requires std::ranges::random_access_range<base_type>
//         { iterator{x} -= y; }

//         friend constexpr difference_type operator-(const iterator& x, const iterator& y)
//         requires std::ranges::random_access_range<base_type>
//         { return x.m_current - y.m_current; }

//     };  // class iterator

//     template <std::ranges::input_range V>
//     requires std::ranges::view<V>
//     template <bool Const>
//     struct enumerate_view<V>::sentinel
//     {
//     private:
//         using base_type = std::conditional_t<Const, const V, V>;
//         std::ranges::sentinel_t<base_type> m_end = std::ranges::sentinel_t<base_type>();
//     public:
//         sentinel() = default;
        
//         constexpr explicit sentinel(std::ranges::sentinel_t<base_type> end) : m_end{ end } { }
        
//         constexpr sentinel(sentinel<!Const> other) 
//         requires Const && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<base_type>> : m_end{ std::move(other.m_end) } { }

//         constexpr std::ranges::sentinel_t<base_type> base() const
//         { return m_end; }

//         // for std::views::iota(1, 2ll) | enumerate  
//         // the x will be iterator<true> and y will be sentinel<false>
//         template <bool OtherConst>
//         friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
//         { return x.m_current == y.m_end; }

//         template <bool OtherConst>
//         friend constexpr std::ranges::range_difference_t<base_type>
//         operator-(const iterator<OtherConst>& x, const sentinel& y)
//         requires std::sized_sentinel_for<std::ranges::sentinel_t<base_type>, std::ranges::iterator_t<base_type>>
//         { return x.m_current - y.m_end; }

//         template <bool OtherConst>
//         friend constexpr std::ranges::range_difference_t<base_type>
//         operator-(const sentinel& x, const iterator<OtherConst>& y)
//         requires std::sized_sentinel_for<std::ranges::sentinel_t<base_type>, std::ranges::iterator_t<base_type>>
//         { return x.m_end - y.m_current; }

//     };

//     struct enumerate_adaptor : range_adaptor_closure
//     {
//         template <std::ranges::viewable_range R>
//         constexpr auto operator() [[nodiscard]] (R&& r) const
//         { return enumerate_view{std::forward<R>(r)}; }
//     };

//     inline constexpr enumerate_adaptor enumerate{};
// }

// namespace std::ranges
// {
//     template <typename R>
//     inline constexpr bool enable_borrowed_range<::leviathan::ranges::enumerate_view<R>> = enable_borrowed_range<R>;
// }
