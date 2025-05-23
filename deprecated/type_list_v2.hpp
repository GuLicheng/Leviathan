#pragma once

#include <leviathan/meta/template_info.hpp> // TypeInfo -> convert type to string
#include <type_traits>
#include <tuple>
#include <algorithm>  // process strings
#include <numeric>  // process index
#include <array>

// decleration
namespace cpp::metaV2
{

    // A basic typelist for storing element as return_type
    template <typename... Ts> struct type_list { };

    template <template <typename...> typename Container, typename List> struct extract;

    template <typename T> struct size_of : std::integral_constant<size_t, sizeof(T)> { };

    // Check whether T is a list such as type_list<Ts...>
    template <typename T> struct is_list;

    template <typename List, typename Target> struct find_first_index_of;

    template <typename List> struct size;

    template <typename List, typename Target> struct contains;

    template <typename List, size_t Idx> struct at;

    template <typename List, template <typename> typename Proj = size_of> struct max;

    template <typename List, template <typename> typename Proj = size_of> struct min;

    template <typename List> struct reverse;

    template <typename List, template <typename> typename Proj = size_of> struct sort;
}


// implement
namespace cpp::metaV2
{
    template <typename T> struct is_list : std::false_type { };

    template <template <typename...> typename List, typename... Ts> 
    struct is_list<List<Ts...>> : std::true_type { };

    template <template <typename...> typename Container, template <typename...> typename List, typename... Ts> 
    struct extract<Container, List<Ts...>> : std::type_identity<Container<Ts...>> { };

    template <template <typename...> typename List, typename Target, typename... Ts>
    struct find_first_index_of<List<Ts...>, Target>
    {
        static constexpr size_t value = [](){
            std::array names { TypeInfo(Ts)... };
            return std::ranges::find(names, TypeInfo(Target)) - names.begin();
        }();
    };
    
    template <template <typename...> typename List, typename... Ts>
    struct size<List<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> { };

    template <typename List, typename Target> 
    struct contains : std::bool_constant<find_first_index_of<List, Target>::value != size<List>::value> { };

    template <template <typename...> typename List, typename... Ts, size_t Idx>
    struct at<List<Ts...>, Idx> : std::tuple_element<Idx, std::tuple<Ts...>> { };

    template <template <typename...> typename List, typename... Ts, template <typename> typename Proj> 
    struct max<List<Ts...>, Proj>
    {
        static_assert(sizeof...(Ts) > 0);
    private:
        static constexpr size_t index = [](){
            std::array names { Proj<Ts>::value... };
            return std::ranges::max_element(names) - names.begin();
        }();
    public:
        using type = typename at<List<Ts...>, index>::type;
        static constexpr auto value = Proj<type>::value;
    };

    template <template <typename...> typename List, typename... Ts, template <typename> typename Proj> 
    struct min<List<Ts...>, Proj>
    {
        static_assert(sizeof...(Ts) > 0);
    private:
        static constexpr size_t index = [](){
            std::array names { Proj<Ts>::value... };
            return std::ranges::min_element(names) - names.begin();
        }();
    public:
        using type = typename at<List<Ts...>, index>::type;
        static constexpr auto value = Proj<type>::value;
    };

} // namespace cpp::metaV2

namespace cpp::metaV2::detail
{

template <typename... Ts>
struct from
{
private:
    template <auto Tuple, size_t...Idx>
    static constexpr auto to_impl(std::index_sequence<Idx...>)
    {
        return std::index_sequence<Tuple[Idx]...>();
    }

    template <auto Tuple>
    static constexpr auto to_()
    {
        return to_impl<Tuple>(std::make_index_sequence<Tuple.size()>());
    }

    template <auto Tuple, size_t...Idx>
    static constexpr auto select_impl(std::index_sequence<Idx...>)
    {
        using List = type_list<Ts...>;
        return type_list<typename at<List, Tuple[Idx]>::type...>();
    }

    template <auto Tuple>
    static constexpr auto select_()
    {
        return select_impl<Tuple>(std::make_index_sequence<Tuple.size()>());
    }

public:

    template <auto Array>
    struct to
    {
        static constexpr auto array2index = to_<Array>();
        using index_type = decltype(array2index);
    };

    template <auto Array>
    struct select
    {
        static constexpr auto array2index = select_<Array>();
        using index_type = decltype(array2index);
    };
};

}  // namespace detail


namespace cpp::metaV2 
{

template <template <typename...> typename List, typename... Ts> 
struct reverse<List<Ts...>> 
{
private:
    static constexpr auto indices = [](){
        std::array<size_t, sizeof...(Ts)> idx;
        std::iota(idx.begin(), idx.end(), 0);
        std::ranges::reverse(idx);
        return idx;
    }();

    using type1 = typename detail::from<Ts...>::template select<indices>::index_type;
public:
    using type = typename extract<List, std::remove_cvref_t<type1>>::type;
};


// template <template <typename...> typename List, typename... Ts, template <typename> typename Proj> 
// struct sort<List<Ts...>, Proj>
// {
//     static_assert(sizeof...(Ts) > 0);
// private:
//     static constexpr auto indices = [](){
//         // struct tow_tuple {  };
//         int i = 0;
//         std::array names { std::make_pair(Proj<Ts>::value..., i++) };
//         std::ranges::stable_sort(names, {}, [](const auto& x) { return x.first; }); // keep ordered
//         return names;
//     }();
//     using type1 = typename detail::from<Ts...>::template select<indices>::index_type;
// public:
//     using type = typename extract<List, std::remove_cvref_t<type1>>::type;
// };

} // namespace cpp::metaV2 



/*


template <std::array Array, std::size_t Length = Array.size()>
consteval auto array_to_index_sequence()
{
    auto return_index_sequence = []<size_t... Idx>(std::index_sequence<Idx...>)
    {
        return std::index_sequence<Array[Idx]...>();
    };
    return return_index_sequence(std::make_index_sequence<Length>());
}

template <std::size_t... Idx>
consteval auto index_sequence_to_array(std::index_sequence<Idx...>)
{
    std::array arr = { Idx... };
    return arr;
}

template <typename TypeList, typename Indices> struct index;

template <typename... Ts, size_t... Idx> 
struct index<std::tuple<Ts...>, std::index_sequence<Idx...>>
{
    using type = std::tuple<std::tuple_element_t<Idx, std::tuple<Ts...>>...>;
};

int main()
{
    constexpr std::array A = { 1, 8, 3 };
    constexpr auto index = array_to_index_sequence<A>();
    constexpr auto array = index_sequence_to_array(std::index_sequence<1, 3, 5>());

    using TL = std::tuple<int, double, bool>;
    using R = typename index<TL, std::index_sequence<0, 2>>::type;

    PrintTypeInfo(R);

    std::cout << "Ok\n";
}



*/








