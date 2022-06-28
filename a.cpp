#include <type_traits>
#include <functional>
#include <cstddef>
#include <iostream>
#include <lv_cpp/collections/internal/meta_helper.hpp>

using namespace leviathan::collections;

struct no_value { };

template <typename K, 
    typename V, 
    typename Compare,
    typename Allocator, 
    bool Duplicate = false, 
    int MaxLevel = 32, 
    int Ratio = 4>
class raw_skip_list
{
    static_assert(!Duplicate, "Not support multi-key now");

    constexpr static bool IsMap = std::is_same_v<V, no_value>;

    constexpr static bool IsTransparent = detail::is_transparent<Compare>;

    template <typename U>
    using key_arg_t = detail::key_arg<IsTransparent, U, K>;

    template <typename T>
    static const auto& s_key(const T& x) 
    {
        if constexpr (IsMap)
            return std::get<0>(x);
        else
            return x;
    }

public:

    using value_type = std::conditional_t<IsMap, std::pair<const K, V>, K>;
    using key_type = K;
    using allocator_type = Allocator;


    raw_skip_list() : m_cmp{ }, m_alloc{ }, m_size{ 0 }, m_header{ }, m_level{ 1 }
    {

    }

private:
public:

    template <typename... Args>
    skip_node* make_skip_node(Args&&... args)
    {

    }

    struct skip_node
    {
        value_type m_val;
        int m_cnt; // node count
        skip_node* m_prev;
        skip_node* m_next[];

        skip_node(skip_node* prev, skip_node* next, int cnt = MaxLevel) noexcept
            : m_prev{ prev }, m_cnt{ cnt }
        {
            std::fill(m_next, m_next + cnt, next);
        }

        template <typename Alloc>
        constexpr static skip_node* allocate(Alloc& alloc, std::size_t x)
        {
            const auto sz = sizeof(skip_node) + sizeof(skip_node*) * x;
            using alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<std::byte>;
            using alloc_traits = typename std::allocator_traits<Alloc>::template rebind_traits<std::byte>;
            alloc_type a{ alloc };
            return reinterpret_cast<skip_node*>(alloc_traits::allocate(a, sz));
        }

        template <typename Alloc>
        constexpr static void deallocate(Alloc& alloc, skip_node* p, std::size_t x)
        {
            const auto sz = sizeof(skip_node) + sizeof(skip_node*) * x;
            using alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<std::byte>;
            using alloc_traits = typename std::allocator_traits<Alloc>::template rebind_traits<std::byte>;
            alloc_type a{ alloc };
            alloc_traits::deallocate(a, reinterpret_cast<std::byte*>(p), sz);
        }


    };

    struct header
    {
        skip_node* m_prev;
        skip_node* m_next[MaxLevel];
        header() : m_prev { nullptr } 
        { std::fill_n(m_next, MaxLevel, nullptr); }
    };


    void show() const 
    {
        std::cout << &m_header << '\n';
        std::cout << m_header.m_prev << '\n';
        for (int i = 0; i < MaxLevel; ++i)
            std::cout << m_header.m_next[i] << '\n';
    }

    using node_type = skip_node;

    [[no_unique_address]] Compare m_cmp;
    [[no_unique_address]] Allocator m_alloc;
    std::size_t m_size;
    header m_header;
    int m_level;


};


template <typename T, typename Compare = std::less<>, typename Allocator = std::allocator<T>>
class skip_set : public raw_skip_list<T, no_value, Compare, Allocator> { };

int main()
{
    skip_set<int> sl;
    sl.show();

}





