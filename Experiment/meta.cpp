#include <stdint.h>

#include <type_traits>
#include <tuple>
#include <utility>
#include <algorithm>
#include <array>

template <typename TypeList, typename Indices>
struct Slice;

template <typename... Ts, size_t... Idx>
struct Slice<std::tuple<Ts...>, std::index_sequence<Idx...>>
{
    using type = std::tuple<
        std::tuple_element_t<Idx, std::tuple<Ts...>>...
    >;
};


template <std::array Array>
struct arr2idx
{
    using type = decltype(
        []<size_t... Idx>(std::index_sequence<Idx...>){
            return std::index_sequence<Array[Idx]...>();
        }(std::make_index_sequence<Array.size()>())
    );
};

template <typename IndexSequence>
struct idx2arr;

template <size_t... Idx>
struct idx2arr<std::index_sequence<Idx...>>
{
    constexpr static std::array value = { Idx... };
};



template <typename TypeList> struct reverse;

template <typename... Ts> 
struct reverse<std::tuple<Ts...>>
{
public:
    using index_sequence = std::index_sequence_for<Ts...>;
    constexpr static auto arr1 = idx2arr<index_sequence>::value;
    constexpr static auto arr2 = [](){
        auto _ = arr1;
        std::reverse(_.begin(), _.end());
        return _;
    }();
    using T = typename arr2idx<arr2>::type;
    using type = typename Slice<std::tuple<int, double, bool>, T>::type;
};

template <typename TypeList> struct sort;

template <typename... Ts> 
struct sort<std::tuple<Ts...>>
{
public:
    using index_sequence = std::index_sequence_for<Ts...>;
    constexpr static auto arr1 = idx2arr<index_sequence>::value;
    constexpr static std::array size_table = { sizeof(Ts)... };
    constexpr static auto arr2 = [](){
        auto _ = arr1;
        std::sort(_.begin(), _.end(), [](const auto idx1, const auto idx2){
            return size_table[idx1] < size_table[idx2];
        });
        return _;
    }();
    using T = typename arr2idx<arr2>::type;
    using type = typename Slice<std::tuple<int, double, bool>, T>::type;
};

int main()
{
    constexpr std::array arr = { 0, 1, 2 };
    using T = typename arr2idx<arr>::type;

    using U = std::index_sequence<0, 2, 1>;
    constexpr auto arr2 = idx2arr<U>::value;

    using V = typename Slice<std::tuple<int, double, bool>, U>::type; 

    using reversed_t = typename sort<std::tuple<int, double, bool>>::type;

}




























