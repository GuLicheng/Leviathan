#pragma once

#include "common.hpp

// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p1899r1.html
namespace leviathan::ranges
{
    template <std::ranges::input_range R>
        requires std::ranges::view<R>
    class stride_view : public std::ranges::view_interface<stride_view<R>>
    {
        template <bool> struct iterator;

        R m_base = R();
        std::ranges::range_difference_t<R> m_stride = 1;


    public:

        template <typename I>
        constexpr I compute_distance(I distance) const
        {
            const auto quotient = distance / static_cast<I>(m_stride);
            const auto remainder = distance % static_cast<I>(m_stride);
            return quotient + static_cast<I>(remainder > 0);
        }

        stride_view() = default;

        constexpr stride_view(R base, std::ranges::range_difference_t<R> stride)
            : m_base(std::move(base)), m_stride(stride) { }

        constexpr R base() const& requires std::copy_constructible<R> { return m_base; }
        constexpr R base() && { return std::move(m_base); }

        constexpr std::ranges::range_difference_t<R> stride() const noexcept
        { 
            return m_stride;
        }
        
        constexpr iterator<false> begin() requires (!detail::simple_view<R>)
        {
            return iterator<false>(*this);
        }

        constexpr iterator<true> begin() const requires (std::ranges::range<const R>)
        {
            return iterator<true>(*this);
        }

        constexpr auto end() requires (!detail::simple_view<R>)
        {
            if constexpr (!std::ranges::bidirectional_range<R> || (std::ranges::sized_range<R> && std::ranges::common_range<R>))
                return iterator<false>(*this, std::ranges::end(m_base), std::ranges::distance(m_base) % m_stride);
            else
                return std::default_sentinel;
        }

        constexpr auto end() const requires (std::ranges::range<const R>)
        {
            if constexpr (!std::ranges::bidirectional_range<R> || (std::ranges::sized_range<R> && std::ranges::common_range<R>))
                return iterator<true>(*this, std::ranges::end(m_base), std::ranges::distance(m_base) % m_stride);
            else
                return std::default_sentinel;
        }

        constexpr auto size() requires (std::ranges::sized_range<R> && !detail::simple_view<R>)
        {
            return compute_distance(std::ranges::size(m_base));
        }

        constexpr auto size() const requires std::ranges::sized_range<const R>
        {
            return compute_distance(std::ranges::size(m_base));
        }

    private:


        template <bool Const>
        struct iterator 
        {
            using parent = detail::maybe_const_t<Const, stride_view>;
            using Base = detail::maybe_const_t<Const, R>;

            parent* m_parent;
            std::ranges::iterator_t<Base> m_current;
            std::ranges::range_difference_t<Base> m_step;

            using value_type = std::ranges::range_value_t<Base>;
            using difference_type = std::ranges::range_difference_t<Base>;
            using iterator_concept = decltype(detail::simple_iterator_concept<R>());

            iterator() = default;

            constexpr explicit iterator(parent& p)
                : m_parent(std::addressof(p)), m_current(std::ranges::begin(p.m_base)), m_step(0) { }

            constexpr iterator(parent& p, std::ranges::iterator_t<Base> end, difference_type step)
                : m_parent(std::addressof(p)), m_current(std::move(end)), m_step(step) { }

            constexpr explicit iterator(iterator<!Const> i)
                requires Const && std::convertible_to<std::ranges::iterator_t<R>, std::ranges::iterator_t<Base>>
                    : m_parent(std::move(i.m_parent)), m_current(std::move(i.m_current)), m_step(i.m_step) { }

            constexpr std::ranges::iterator_t<Base> base() const;

            constexpr decltype(auto) operator*() const
            {
                return *m_current;
            }

            constexpr iterator& operator++()
            {
                return advance(1);
            }

            constexpr auto operator++(int)
            {
                if constexpr (std::ranges::forward_range<Base>)
                {
                    auto temp = *this;
                    ++*this;
                    return temp;
                }
                else
                {
                    (void)(this->operator++());
                }
            }

            constexpr iterator& operator--() requires std::ranges::bidirectional_range<Base>
            {
                return advance(-1);
            }

            constexpr iterator operator--(int) requires std::ranges::bidirectional_range<Base>
            {
                auto temp = *this;
                ++*this;
                return temp;
            }


            constexpr iterator& operator+=(difference_type n)  
                requires std::ranges::random_access_range<Base>
            {
                return advance(n);
            }

            constexpr iterator& operator-=(difference_type n)  
                requires std::ranges::random_access_range<Base>
            {
                return advance(-n);
            }

            constexpr decltype(auto) operator[](difference_type n) const
                requires std::ranges::random_access_range<Base>
            {
                return *(*this + n);
            }

            constexpr friend iterator operator+(const iterator& x, difference_type n)
                requires std::ranges::random_access_range<Base>
            {
                auto r = x;
                r += n;
                return r;
            }

            constexpr friend iterator operator+(difference_type n, const iterator& x)
                requires std::ranges::random_access_range<Base>
            {
                return x + n;
            }

            constexpr friend iterator operator-(const iterator& x, difference_type n)
                requires std::ranges::random_access_range<Base>
            {
                auto r = x;
                r -= n;
                return r;
            }

            constexpr friend difference_type operator-(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return x.m_parent->compute_distance(x.m_current - y.m_current);
            }

            constexpr friend bool operator==(const iterator& x, const iterator& y)
                requires std::equality_comparable<std::ranges::iterator_t<Base>>
            {
                return x.m_current == y.m_current;
            }

            constexpr friend bool operator<(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return x.m_current < y.m_current;
            }

            constexpr friend bool operator>(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return y < x;
            }

            constexpr friend bool operator<=(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return !(x > y);    
            }

            constexpr friend bool operator>=(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base>
            {
                return !(x < y);
            }

            constexpr friend auto operator<=>(const iterator& x, const iterator& y)
                requires std::ranges::random_access_range<Base> && std::three_way_comparable<std::ranges::iterator_t<Base>>
            {
                return x.m_current <=> y.m_current;
            }

            constexpr friend std::ranges::range_rvalue_reference_t<R> iter_move(const iterator& i)
            noexcept(noexcept(std::ranges::iter_move(i.m_current)))
            {
                return std::ranges::iter_move(i.m_current);
            }

            constexpr friend void iter_swap(const iterator& x, const iterator& y)
            noexcept(noexcept(std::ranges::iter_swap(x.m_current, y.m_current)))
                requires std::indirectly_swappable<std::ranges::iterator_t<R>>
            {
                std::ranges::iter_swap(x.m_current, y.m_current);
            }

        private:

            constexpr iterator& advance(difference_type n)
            {
                if constexpr (!std::ranges::bidirectional_range<parent>) 
                {
                    std::ranges::advance(m_current, n * m_parent->m_stride, std::ranges::end(m_parent->m_base));
                }
                else 
                {
                    if (n > 0) 
                    {
                        auto remaining = std::ranges::advance(m_current, n * m_parent->m_stride, std::ranges::end(m_parent->m_base));
                        m_step = m_parent->m_stride - remaining;
                    }
                    else if (n < 0)
                    { 
                        auto stride = m_step == 0 ? n * m_parent->m_stride
                                                : (n + 1) * m_parent->m_stride - m_step;
                        std::ranges::advance(m_current, stride);
                        m_step= 0;
                    }
                }
                return *this;
            }

        };

    };

    template <typename R>
    stride_view(R&&, std::ranges::range_difference_t<R>) -> stride_view<std::views::all_t<R>>;

    template <typename R, typename N>
    concept can_stride = requires 
    {
        stride_view(std::declval<R>(), std::declval<N>());
        // requires std::same_as<N, std::ranges::range_difference_t<R>>;
    };

    struct stride_adaptor : range_adaptor<stride_adaptor>
    {
        using range_adaptor<stride_adaptor>::operator();
        template <std::ranges::viewable_range R, typename N>
            requires can_stride<R, N>
        constexpr auto operator()(R&& r, N n) const
        {
            return stride_view((R&&)r, n);
        }
    };

    inline constexpr stride_adaptor stride{};

}

namespace std::ranges
{
    template <typename R>
    inline constexpr bool enable_borrowed_range<::leviathan::ranges::stride_view<R>> 
        = std::ranges::forward_range<R> && enable_borrowed_range<R>;
}
