#include <variant>
#include <tuple>
#include <functional>
#include <array>
#include <algorithm>

#include <lv_cpp/meta/type_list.hpp>

// Arr2IndexSeq IndexSeq2Arr

template <typename Indices1, typename Indices2>
struct next_index_sequence;


template <std::array, typename IndexSeq>
struct Arr2IdxImpl;

template <std::array value, std::size_t... Idx>
struct Arr2IdxImpl<value, std::index_sequence<Idx...>>
{
    using type = std::index_sequence<
        value[Idx]...
    >;
};

template <std::array value>
struct Arr2Idx
{
    using type = typename Arr2IdxImpl<value, std::make_index_sequence<value.size()>>::type;
};

template <typename T>
struct Idx2Arr;

template <std::size_t... Idx>
struct Idx2Arr<std::index_sequence<Idx...>>
{
    constexpr static std::array value = { Idx... };
};


int main()
{

}
