#pragma once

#include "common.hpp"

// ranges.chunk_by_view
namespace leviathan::ranges
{   
    template <std::ranges::forward_range V, std::indirect_binary_predicate<std::ranges::iterator_t<V>, std::ranges::iterator_t<V>> Pred>
        requires std::ranges::view<V> && std::is_object_v<Pred>
    class chunk_by_view : public std::ranges::view_interface<chunk_by_view<V, Pred>>
    {
        V m_base = V();
        std::optional<Pred> m_pred = Pred();

        enum class store_type { None, Iter, Offset };

        //  For forward_range, cache iterator, for random_access_range, cache offset, View otherwise 
        constexpr static store_type st = []{
            if constexpr (std::ranges::random_access_range<V> && 
                (sizeof(std::ranges::range_difference_t<V>) <= sizeof(std::ranges::iterator_t<V>)))
                return store_type::Offset;
            else if constexpr (std::ranges::forward_range<V>)
                return store_type::Iter;
            else
                return store_type::None;
        }();

        using cache_type = decltype([]{
            if constexpr (st == store_type::None)
                return empty_class();
            else if constexpr (st == store_type::Iter)
                return std::optional<std::ranges::iterator_t<V>>();
            else
                return std::optional<std::ranges::range_difference_t<V>>();
        }());

        [[no_unique_address]] cache_type m_cache;

        struct iterator
        {
            chunk_by_view* m_parent = nullptr;
            std::ranges::iterator_t<V> m_current = std::ranges::iterator_t<V>();
            std::ranges::iterator_t<V> m_next = std::ranges::iterator_t<V>();

            constexpr iterator(chunk_by_view& parent, std::ranges::iterator_t<V> current, std::ranges::iterator_t<V> next)
                : m_parent(std::addressof(parent)), m_current(std::move(current)), m_next(std::move(next)) { }

            using value_type = std::ranges::subrange<std::ranges::iterator_t<V>>;
            using difference_type = std::ranges::range_difference_t<V>;
            using iterator_category = std::input_iterator_tag;
            using iterator_concept = decltype([]{
                if constexpr (std::ranges::bidirectional_range<V>)
                    return std::bidirectional_iterator_tag();
                else
                    return std::forward_iterator_tag();
            }());

            iterator() = default;
            
            constexpr value_type operator*() const
            {
                assert(m_current != m_next);
                return std::ranges::subrange(m_current, m_next);
            }

            constexpr iterator& operator++()
            {
                m_current = m_next;
                m_next = m_parent->find_next(m_current);
                return *this;
            }

            constexpr iterator operator++(int)
            {
                auto temp = *this;
                ++*this;
                return temp;
            }
            
            constexpr iterator& operator--() requires std::ranges::bidirectional_range<V>
            {
                m_next = m_current;
                m_current = m_parent->find_prev(m_next);
                return *this;
            }
            
            constexpr iterator operator--(int) requires std::ranges::bidirectional_range<V>
            {
                auto temp = *this;
                --*this;
                return temp;
            }

            friend constexpr bool operator==(const iterator& x, const iterator& y)
            {
                return x.m_current == y.m_current;
            }

            friend constexpr bool operator==(const iterator& x, std::default_sentinel_t)
            {
                return x.m_current == x.m_next;
            }

        };

    public:
        chunk_by_view() requires std::default_initializable<V> && std::default_initializable<Pred> = default;
        
        constexpr explicit chunk_by_view(V base, Pred pred) 
            : m_base(std::move(base)), m_pred(std::move(pred)) { }

        constexpr V base() const& requires std::copy_constructible<V> { return m_base; }
        constexpr V base() && { return std::move(base); }

        constexpr const Pred& pred() const { return *m_pred; }
        
        constexpr iterator begin()
        {
            // In order to provide the amortized constant-time complexity required by the range concept
            // this function caches the result within chunk_by_view for use on subsequent calls.
            // return iterator(*this, std::ranges::begin(m_base), find_next(std::ranges::begin(m_base)));

            assert(m_pred.has_value());

            if constexpr (st == store_type::None)
            {
                return iterator(*this, std::ranges::begin(m_base), find_next(std::ranges::begin(m_base)));
            }
            else
            {
                if (!m_cache.has_value())
                {
                    auto it = find_next(std::ranges::begin(m_base));
                    if constexpr (st == store_type::Offset)
                        m_cache.emplace(std::ranges::distance(std::ranges::begin(m_base), it));
                    else    
                        m_cache.emplace(std::move(it));
                }

                if constexpr (st == store_type::Offset)
                    return iterator(*this, std::ranges::begin(m_base), std::ranges::begin(m_base) + *m_cache);
                else
                    return iterator(*this, std::ranges::begin(m_base), *m_cache);
            }

        }

        constexpr auto end()
        {
            if constexpr (std::ranges::common_range<V>)
                return iterator(*this, std::ranges::end(m_base), std::ranges::end(m_base));
            else
                return std::default_sentinel;
        }

        constexpr std::ranges::iterator_t<V> find_next(std::ranges::iterator_t<V> current)
        {
            
            auto not_pred = [this]<typename L, typename R>(L&& l, R&& r) {
                return !std::invoke(this->m_pred.value(), (L&&)l, (R&&)r);
            };

            return std::ranges::next(std::ranges::adjacent_find(current, std::ranges::end(m_base), not_pred), 
                1, std::ranges::end(m_base));
        }

        constexpr std::ranges::iterator_t<V> find_prev(std::ranges::iterator_t<V> current)
            requires std::ranges::bidirectional_range<V>
        {
            assert(m_pred.has_value() && current != std::ranges::begin(m_base));
            // Return an iterator i in the range [range::begin(base_), current] such that:
            // ranges::adjacent_find(i, current, not_fn(ref(*pred))) is equal to current; and
            // if i is not equal to ranges::begin(base_), then bool(invoke(*pred_, *ranges::prev(i), *i)) is false.

            std::ranges::reverse_view rv { std::ranges::subrange(std::ranges::begin(m_base), current) };
            const auto rev_not_pred = [this]<typename L, typename R>(L&& l, R&& r) {
                return !std::invoke(this->m_pred.value(), (R&&)r, (L&&)l);
            };
            const auto after_prev = std::ranges::adjacent_find(rv, rev_not_pred);
            return std::ranges::prev(after_prev.base(), 1, std::ranges::begin(m_base));
        }
    };

    template <typename R, typename Pred>
    chunk_by_view(R&&, Pred) -> chunk_by_view<std::views::all_t<R>, Pred>;

    template <typename R, typename Pred>
    concept can_chunk_by = requires 
    {
        chunk_by_view(std::declval<R>(), std::declval<Pred>());
    };

    struct chunk_by_adaptor : range_adaptor<chunk_by_adaptor>
    {
        using range_adaptor<chunk_by_adaptor>::operator();

        template <std::ranges::viewable_range R, typename Pred>
            requires can_chunk_by<R, Pred>
        constexpr auto operator()(R&& r, Pred&& pred) const
        {
            return chunk_by_view((R&&)r, (Pred&&)pred);
        }
    };

    inline constexpr chunk_by_adaptor chunk_by{};

}
