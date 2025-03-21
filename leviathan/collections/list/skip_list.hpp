#pragma once

#include <leviathan/collections/common.hpp>
#include <leviathan/collections/container_interface.hpp>
#include <leviathan/utils/layout.hpp>
#include <leviathan/allocators/adaptor_allocator.hpp>
#include <leviathan/collections/list/skip_node.hpp>
#include <random>
#include <cstddef>

namespace leviathan::collections
{
    
/**
 * @brief A skiplist implementation.
 * 
 * @param KeyValue Extractor extract key from value. identity<T> for set and select1st<K, V> for map.
 * @param Compare
 * @param Allocator
 * @param UniqueKey True for set/map and False for multiset/multimap.
 * @param RandomNumberGenerator Random generator to generate random numbers.
 * @param MaxLevel Max level of node.
 * @param Ratio Reciprocal of probability.
*/
template <typename KeyValue, 
    typename Compare, 
    typename Allocator, 
    bool UniqueKey, 
    typename RandomEngine = std::mt19937,
    typename SeedGenerator = std::random_device,
    int MaxLevel = 24, 
    int Ratio = 4>
class skip_list : public container_interface
{
    static_assert(Ratio > 1);
    static_assert(std::is_unsigned_v<typename RandomEngine::result_type>);

    static constexpr auto p = std::lerp(RandomEngine::min(), RandomEngine::max(), 1.0 / Ratio);

    inline static RandomEngine random = RandomEngine(SeedGenerator()());

    static int get_level()
    {
        int level = 1;
        for (; random() < p; ++level);
        return std::min(MaxLevel, level);
    }

public:

    using key_value = KeyValue;
    using key_type = typename key_value::key_type;
    using value_type = typename key_value::value_type;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using value_compare = Compare;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    using size_type = size_t;
    using allocator_type = Allocator;
    using iterator = skip_iterator<key_value>;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
public:

    using list_node = skip_node<value_type>;
    using node_layout_type = typename list_node::node_layout_type;
    using node_allocator = adaptor_allocator<Allocator, std::byte>;

    template <typename U> 
    using key_arg_t = detail::key_arg<detail::transparent<Compare>, U, key_type>;

    static constexpr bool IsNothrowMoveConstruct = 
                std::is_nothrow_move_constructible_v<Compare> 
                && typename Allocator::is_always_equal();

    static constexpr bool IsNothrowMoveAssign = 
                std::is_nothrow_move_assignable_v<Compare> 
                && typename Allocator::is_always_equal();

    static constexpr bool IsNothrowSwap = 
                std::is_nothrow_swappable_v<Compare> 
                && typename Allocator::is_always_equal();

    iterator begin() 
    {
        return iterator(header()->pointers[1]);
    }

    const_iterator begin() const
    {
        return std::make_const_iterator(const_cast<skip_list&>(*this).begin());
    }

    iterator end()
    {
        return iterator(header());
    }

    const_iterator end() const
    {
        return std::make_const_iterator(const_cast<skip_list&>(*this).end());
    }

    size_type size() const
    {
        return m_size;
    }

    bool empty() const
    {
        return size() == 0;
    }

    allocator_type get_allocator() const
    {
        return allocator_type(m_alloc);
    }

    size_type max_size() const
    {
        using AllocTraits = std::allocator_traits<Allocator>;
        return AllocTraits::max_size(m_alloc);
    }

    // Observers
    key_compare key_comp() const
    {
        return m_cmp;
    }

    value_compare value_comp() const
    {
        return m_cmp;
    }

    int level() const
    {
        return m_level;
    }

    skip_list(const Compare& compare, const Allocator& allocator)
        : m_cmp(compare), m_alloc(allocator)
    {
        m_level = 1;
        m_size = 0;
        m_header = alloc_node(MaxLevel);
        reset_header();
    }

    skip_list() : skip_list(Compare(), Allocator()) { }

    ~skip_list() 
    {
        clear();
    }

    void clear()
    {
        reset();
    }

    template <typename... Args>
    void construct_node(list_node* node, Args&&... args)
    {
        using AllocTraits = std::allocator_traits<Allocator>; 

        try
        {
            AllocTraits::construct(m_alloc, node->value_ptr(), (Args&&) args...);
        }
        catch (...)
        {
            dealloc_node(node);
            throw;
        }
    }

    void destroy_node(list_node* node)
    {
        using AllocTraits = std::allocator_traits<Allocator>;
        AllocTraits::destroy(m_alloc, node->value_ptr());
    }

    [[nodiscard]] list_node* alloc_node(int count)
    {
        auto size = node_layout_type(1, 1, 1 + count).template alloc_size();
        auto node = static_cast<list_node*>(
            static_cast<void*>(node_allocator::allocate(m_alloc, size))
        );

        node->count_ref() = count;
        return node;
    }

    void dealloc_node(list_node* node)
    {
        auto size = node->alloc_size();
        node_allocator::deallocate(m_alloc, node->self_ptr(), size);
    }

    void drop_node(list_node* node)
    {
        destroy_node(node);
        dealloc_node(node);
    }

    template <typename... Args>
    list_node* create_node(Args&&... args)
    {
        auto node = alloc_node(get_level());
        construct_node(node, (Args&&) args...);
        return node;
    }

    void reset_header()
    {
        assert(header()->count() == MaxLevel);
        header()->reset(m_header, m_header);
    }

    void reset()
    {
        auto node = header()->pointers()[0];

        while (node != header())
        {
            auto next = node->pointers()[0];
            drop_node(node);
            node = next;
        }

        m_level = 1;
        m_size = 0;
        reset_header();
    }

    list_node* header()
    { return m_header; }

    const list_node* header() const 
    { return m_header; }

    [[no_unique_address]] Compare m_cmp;
    [[no_unique_address]] Allocator m_alloc;
    int m_level;
    list_node* m_header;
    size_type m_size;

};

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using skip_set = skip_list<identity<T>, Compare, Allocator, true>;


} // namespace leviathan::collections

