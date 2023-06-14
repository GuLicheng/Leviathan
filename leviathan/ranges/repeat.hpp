// #pragma once

// #include "common.hpp"

// // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2474r2.html
// namespace leviathan::ranges
// {
//     template <std::copy_constructible W, std::semiregular Bound = std::unreachable_sentinel_t>
//     requires (std::is_object_v<W> && std::same_as<W, std::remove_cvref_t<W>> && (std::integral<Bound> || std::same_as<Bound, std::unreachable_sentinel_t>))
//     class repeat_view : public std::ranges::view_interface<repeat_view<W, Bound>>
//     {
//     private:
//         struct iterator;

//         std::optional<W> m_value = std::optional<W>();
//         Bound m_bound = Bound();

//     public:
//         repeat_view() requires std::default_initializable<W> = default;

//         constexpr explicit repeat_view(const W& value, Bound bound = Bound()) 
//             : m_value(value), m_bound(bound) { }

//         constexpr explicit repeat_view(W&& value, Bound bound = Bound()) 
//             : m_value(std::move(value)), m_bound(bound) { }

//         template <typename... WArgs, typename... BoundArgs>
//         requires std::constructible_from<W, WArgs...> && std::constructible_from<Bound, BoundArgs...>
//         constexpr explicit repeat_view(std::piecewise_construct_t, std::tuple<WArgs...> value_args, std::tuple<BoundArgs...> bound_args = std::tuple<>{}) 
//             : m_value(std::make_from_tuple<std::optional<W>>(
//                 std::tuple_cat(
//                     std::make_tuple(std::in_place), 
//                     std::move(value_args)
//                 ))), 
//               m_bound(std::make_from_tuple<Bound>(std::move(bound_args))) { }


//         constexpr iterator begin() const 
//         { return iterator(std::addressof(*m_value)); }

//         constexpr iterator end() const requires (!std::same_as<Bound, std::unreachable_sentinel_t>)
//         { return iterator(std::addressof(*m_value), m_bound); }
        
//         constexpr std::unreachable_sentinel_t end() const noexcept
//         { return std::unreachable_sentinel; }
        
//         constexpr auto size() const requires (!std::same_as<Bound, std::unreachable_sentinel_t>)
//         { return std::make_unsigned_t<Bound>(m_bound); }

//     };

//     template <std::copy_constructible W, std::semiregular Bound>
//     requires (std::is_object_v<W> && std::same_as<W, std::remove_cvref_t<W>> && (std::integral<Bound> || std::same_as<Bound, std::unreachable_sentinel_t>))
//     class repeat_view<W, Bound>::iterator
//     {
//         using index_type = std::conditional_t<std::same_as<Bound, std::unreachable_sentinel_t>, std::ptrdiff_t, Bound>;
//         const W* m_value = nullptr;
//         index_type m_current = index_type();
//     public:


//         using iterator_concept = std::random_access_iterator_tag;
//         using iterator_category = std::random_access_iterator_tag;
//         using value_type = W;
//         using difference_type = std::conditional_t<std::integral<index_type>, index_type, std::ptrdiff_t>;


//         constexpr iterator() = default;

//         constexpr explicit iterator(const W* value, index_type b = index_type())
//             : m_value(value), m_current(b) { }

//         constexpr const W& operator*() const noexcept
//         { return *m_value; }

//         constexpr iterator& operator++()
//         { 
//             ++m_current;
//             return *this;
//         }

//         constexpr iterator operator++(int)
//         {
//             auto temp = *this;
//             ++*this;
//             return temp;
//         }

//         constexpr iterator& operator--()
//         {
//             --m_current;
//             return *this;
//         }

//         constexpr iterator operator--(int)
//         {
//             auto temp = *this;
//             --*this;
//             return temp;
//         }


//         constexpr iterator& operator+=(difference_type n)
//         {
//             m_current += n;
//             return *this;
//         }

//         constexpr iterator& operator-=(difference_type n)
//         {
//             m_current -= n;
//             return *this;
//         }

//         constexpr const W& operator[](difference_type n) const noexcept
//         { return *(*this + n); }

//         friend constexpr bool operator==(const iterator& lhs, const iterator& rhs)
//         { return lhs.m_current == rhs.m_current; }

//         friend constexpr bool operator<=>(const iterator& lhs, const iterator& rhs)
//         { return lhs.m_current <=> rhs.m_current; }

//         friend constexpr iterator operator+(iterator i, difference_type n)
//         { 
//             i += n;
//             return i;
//         }

//         friend constexpr iterator operator+(difference_type n, iterator i)
//         { return i + n; }

//         friend constexpr iterator operator-(iterator i, difference_type n)
//         { 
//             i -= n;
//             return i;
//         }

//         friend constexpr difference_type operator-(const iterator& x, const iterator& y)
//         { return static_cast<difference_type>(x.m_current) - static_cast<difference_type>(y.m_current); }

//     };
    
//     struct repeat_factory
//     {
//         template <typename W, std::semiregular Bound>
//         requires std::movable<W> && std::copy_constructible<W>
//         constexpr auto operator()(W w, Bound bound) const 
//         { return repeat_view(std::move(w), bound); }
    
//         template <typename W>
//         requires std::movable<W> && std::copy_constructible<W>
//         constexpr auto operator()(W w) const
//         { return repeat_view(std::move(w), std::unreachable_sentinel); } 
//     };

//     inline constexpr repeat_factory repeat{}; 

//     struct cycle_adaptor : range_adaptor_closure
//     {
//         template <typename R>
//         constexpr auto operator()(R&& r) const
//         {
//             return repeat((R&&)r) | std::views::join;
//         }
//     };

//     inline constexpr cycle_adaptor cycle{};

// }
