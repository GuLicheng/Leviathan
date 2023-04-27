#include <tuple>
#include <bitset>
#include <iostream>
#include <cstddef>
#include <array>
#include <algorithm>
#include <bit>
#include <span>
#include <vector>
#include <assert.h>

namespace detail
{
    template <typename T, typename... Ts>
    consteval size_t find_first_index()
    {
        static_assert(std::disjunction_v<std::is_same<T, Ts>...>);
        std::array result = { std::is_same_v<T, Ts>... };
        return std::ranges::distance(
            result.begin(), 
            std::ranges::find(result, 1));
    }

    constexpr size_t align(size_t n, size_t m) 
    { 
        assert(std::popcount(m) == 1 && "m should be power of two.");
        return (n + m - 1) & ~(m - 1); 
    }

    template <auto>
    using to_size_t = size_t;

} // namespace detail

template <typename TypeList, typename SizeSequence> 
struct layout_impl;

template <typename... Ts, size_t... Sizes> 
struct layout_impl<std::tuple<Ts...>, std::index_sequence<Sizes...>>
{
    constexpr static auto NumTypes = sizeof...(Ts);
    constexpr static auto NumSizes = sizeof...(Sizes);

    static_assert(NumTypes > 0, "Internal error");

    template <typename T>
    static constexpr size_t type_index() 
    {
        return detail::find_first_index<T, Ts...>();
    }
   
    static constexpr size_t alignment() 
    {
        return std::ranges::max({ alignof(Ts)... });
    }

    template <size_t N>
    constexpr size_t size() const 
    {
        static_assert(N < NumSizes);
        return m_sizes[N];
    }

    constexpr std::array<size_t, NumSizes> sizes() const
    {
        return { size<Sizes>()... };
    }

    template <size_t N>
    constexpr size_t offset() const
    {
        static_assert(N < NumSizes);
        if constexpr (N == 0)
        {
            return 0;
        }
        else
        {
            const auto last_type_size = sizeof(std::tuple_element_t<N - 1, std::tuple<Ts...>>);
            const auto current_type_alignment = alignof(std::tuple_element_t<N, std::tuple<Ts...>>);
            return detail::align(offset<N - 1>() + last_type_size * m_sizes[N - 1], current_type_alignment);
        }
    }

    constexpr std::array<size_t, NumSizes> offsets() const
    {
        return { offset<Sizes>()... };
    }

    constexpr size_t alloc_size() const
    {
        return offset<NumTypes - 1>() + sizeof(std::tuple_element_t<NumTypes - 1, std::tuple<Ts...>>) * m_sizes[NumTypes - 1];
    }

    template <size_t N, typename Char>
    constexpr std::tuple_element_t<N, std::tuple<Ts...>>* pointer(Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        assert(reinterpret_cast<uintptr_t>(p) % alignment() == 0);
        return reinterpret_cast<std::tuple_element_t<N, std::tuple<Ts...>>*>(p + offset<N>());
    } 

    template <size_t N, typename Char>
    constexpr const std::tuple_element_t<N, std::tuple<Ts...>>* pointer(const Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        assert(reinterpret_cast<uintptr_t>(p) % alignment() == 0);
        return reinterpret_cast<const std::tuple_element_t<N, std::tuple<Ts...>>*>(p + offset<N>());
    } 

    template <size_t N, typename Char>
    constexpr std::span<std::tuple_element_t<N, std::tuple<Ts...>>> slice(Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        auto start = pointer<N>(p);
        return std::span<std::tuple_element_t<N, std::tuple<Ts...>>>(start, start + size<N>());
    } 

    template <size_t N, typename Char>
    constexpr std::span<const std::tuple_element_t<N, std::tuple<Ts...>>> slice(const Char* p) const
    {
        static_assert(sizeof(Char) == 1);
        auto start = pointer<N>(p);
        return std::span<const std::tuple_element_t<N, std::tuple<Ts...>>>(start, start + size<N>());
    } 

    constexpr explicit layout_impl(detail::to_size_t<Sizes>... sizes)
      : m_sizes{ sizes... } {}

    std::array<size_t, NumSizes> m_sizes;
};

template <typename... Ts>
struct layout : layout_impl<
    std::tuple<Ts...>, std::make_index_sequence<sizeof...(Ts)>>
{
    using base = layout_impl<std::tuple<Ts...>, std::make_index_sequence<sizeof...(Ts)>>;
    using base::base;
};

void test_layout()
{
    static_assert(layout<int32_t>(3).offset<0>() == 0);
    static_assert(layout<int32_t>(3).offsets() == std::array<size_t, 1>{ 0 });
    static_assert(layout<int32_t>(3).alloc_size() == 12);
    static_assert(layout<int32_t>(3).size<0>() == 3);

    static_assert(layout<int32_t, int32_t>(3, 5).offset<0>() == 0);
    static_assert(layout<int32_t, int32_t>(3, 5).offset<1>() == 12);
    static_assert(layout<int32_t, int32_t>(3, 5).offsets() == std::array<size_t, 2>{ 0, 12 });
    static_assert(layout<int32_t, int32_t>(3, 5).alloc_size() == 32);
    static_assert(layout<int32_t, int32_t>(3, 5).size<0>() == 3);
    static_assert(layout<int32_t, int32_t>(3, 5).size<1>() == 5);
    
    static_assert(layout<int8_t, int32_t , std::pair<int64_t, int64_t>>(5, 3, 1).offset<0>() == 0);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(5, 3, 1).offset<1>() == 8);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(5, 3, 1).offset<2>() == 24);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(5, 3, 1).offsets() == std::array<size_t, 3>{ 0, 8, 24 });
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(3, 5, 7).alloc_size() == 136);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(3, 5, 7).size<0>() == 3);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(3, 5, 7).size<1>() == 5);
    static_assert(layout<int8_t, int32_t, std::pair<int64_t, int64_t>>(3, 5, 7).size<2>() == 7);

    static_assert(layout<int8_t>::alignment() == 1);
    static_assert(layout<int32_t>::alignment() == 4);
    static_assert(layout<int64_t>::alignment() == 8);
    static_assert(layout<int8_t, int32_t, int64_t>::alignment() == 8);
    static_assert(layout<int8_t, int64_t, int32_t>::alignment() == 8);
    static_assert(layout<int32_t, int8_t, int64_t>::alignment() == 8);
    static_assert(layout<int32_t, int64_t, int8_t>::alignment() == 8);
    static_assert(layout<int64_t, int8_t, int32_t>::alignment() == 8);
    static_assert(layout<int64_t, int32_t, int8_t>::alignment() == 8);

    {
        using L1 = layout<int32_t, int32_t>;
        alignas(max_align_t) unsigned char p[100] = {};
        assert(reinterpret_cast<unsigned char*>(L1(3, 5).pointer<0>(p)) == p);
        assert(reinterpret_cast<unsigned char*>(L1(3, 5).pointer<1>(p)) == p + 12);
    }

    {
        using L2 = layout<int8_t, int32_t, std::pair<int64_t, int64_t>>;
        alignas(max_align_t) const unsigned char p[100] = {};
        assert(reinterpret_cast<const unsigned char*>(L2(5, 3, 1).pointer<0>(p)) == p);
        assert(reinterpret_cast<const unsigned char*>(L2(5, 3, 1).pointer<1>(p)) == p + 8);
        assert(reinterpret_cast<const unsigned char*>(L2(5, 3, 1).pointer<2>(p)) == p + 24);
    }

    {
        using L3 = layout<int8_t, int32_t, std::pair<int64_t, int64_t>>;
        alignas(max_align_t) const unsigned char p[100] = {};
        assert(L3(3, 5, 7).slice<0>(p).size() == 3);
        assert(L3(3, 5, 7).slice<1>(p).size() == 5);
        assert(L3(3, 5, 7).slice<2>(p).size() == 7);
    }

}

void struct_test()
{
    using T = layout<int, double>;
    T l(3, 4);

    std::cout << l.type_index<double>() << '\n';
    std::cout << l.type_index<int>() << '\n';

    std::cout << l.size<0>() << '\n';
    std::cout << l.size<1>() << '\n';
    
    std::cout << l.offset<0>() << '\n';
    std::cout << l.offset<1>() << '\n';

    std::cout << l.alloc_size() << '\n';

    struct F
    {
        int a[3];
        double b[4];
        // alignas(int) char* a[sizeof(int) * 3];
        // alignas(double) char* b[sizeof(double) * 3];

        void show() const
        {
            std::cout << a[0] << a[1] << a[2] << '-'
                << b[1] << b[2] << b[3] << b[4] << '\n';
        }
    };


    std::cout << l.alignment() << '\n';
    std::cout << alignof(F) << '\n';

    unsigned char* p = new unsigned char[l.alloc_size()];
    F* f = reinterpret_cast<F*>(p);

    f->a[0] = 0;
    f->a[1] = 1;
    f->a[2] = 2;

    f->b[0] = 0;
    f->b[1] = -1;
    f->b[2] = -2;
    f->b[0] = -3;

    int* ints1 = l.pointer<0>(p);
    double* doubles1 = l.pointer<1>(p);

    auto ints2 = l.slice<0>(p);
    auto doubles2 = l.slice<1>(p);

    std::cout << ints2[2] << doubles2[2] << '\n';

}


int main(int argc, char const *argv[])
{

    layout<int8_t, int16_t, double> l(1, 1, 1);

    std::cout << l.offset<0>() << std::endl;
    std::cout << l.offset<1>() << std::endl;

    std::vector<int> vc;
    vc.insert(vc.begin(), 1);

    return 0;   
}
