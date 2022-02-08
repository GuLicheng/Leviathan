#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/string/fixed_string.hpp>
#include "type_list_v2.hpp"
#include <iostream>
#include <array>
#include <algorithm>
#include <tuple>
#include <numeric>

/*
    From<List>::to<Index>(); -> types
*/
namespace meta = leviathan::metaV2;

template <typename... Ts>
struct From
{
private:
    template <auto Tuple, size_t...Idx>
    constexpr static auto to_impl(std::index_sequence<Idx...>)
    {
        return std::index_sequence<Tuple[Idx]...>();
    }

    template <auto Tuple>
    constexpr static auto to()
    {
        return to_impl<Tuple>(std::make_index_sequence<Tuple.size()>());
    }

    template <auto Tuple, size_t...Idx>
    constexpr static auto select_impl(std::index_sequence<Idx...>)
    {
        using List = meta::type_list<Ts...>;
        return meta::type_list<typename meta::at<List, Tuple[Idx]>::type...>();
    }

    template <auto Tuple>
    constexpr static auto select()
    {
        return select_impl<Tuple>(std::make_index_sequence<Tuple.size()>());
    }

public:

    template <auto Array>
    struct To
    {
        constexpr static auto array2index = to<Array>();
        using index_type = decltype(array2index);
    };

    template <auto Array>
    struct Select
    {
        constexpr static auto array2index = select<Array>();
        using index_type = decltype(array2index);
    };

};


template <typename List> struct Reverse;

template <template <typename...> typename List, typename... Ts> 
struct Reverse<List<Ts...>> 
{
    constexpr static auto indices = [](){
        std::array<size_t, sizeof...(Ts)> idx;
        std::iota(idx.begin(), idx.end(), 0);
        std::ranges::reverse(idx);
        return idx;
    }();

    using type1 = typename From<Ts...>::template Select<indices>::index_type;
    using type = typename meta::extract<List, std::remove_cvref_t<type1>>::type;
};




int main()
{
    using List = std::tuple<int, double, bool, const char*>;
    
    static_assert(meta::size<List>::value == 4);
    static_assert(meta::contains<List, int>::value);
    static_assert(!meta::contains<List, float>::value);
    static_assert(meta::is_list<List>::value);
    static_assert(!meta::is_list<int>::value);
    static_assert(meta::max<List>::value == sizeof(double));
    static_assert(meta::min<List>::value == sizeof(bool));
    static_assert(std::is_same_v<typename meta::at<List, 0>::type, int>);
    static_assert(std::is_same_v<typename meta::max<List>::type, double>);
    static_assert(std::is_same_v<typename meta::min<List>::type, bool>);

    constexpr std::array indices = { 2, 0, 1 };
    auto i = From<int, double, bool>::To<indices>::array2index;
    using U = From<int, double, bool>::To<indices>::index_type;
    using T = typename Reverse<List>::type;
    PrintTypeInfo(T);
}

