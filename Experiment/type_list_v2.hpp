#pragma once

#include <lv_cpp/meta/template_info.hpp> // TypeInfo -> convert type to string
#include <type_traits>
#include <tuple>
#include <algorithm>  // process strings
#include <numeric>  // process index
#include <array>

// decleration
namespace leviathan::metaV2
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
namespace leviathan::metaV2
{
    template <typename T> struct is_list : std::false_type { };

    template <template <typename...> typename List, typename... Ts> 
    struct is_list<List<Ts...>> : std::true_type { };

    template <template <typename...> typename Container, template <typename...> typename List, typename... Ts> 
    struct extract<Container, List<Ts...>> : std::type_identity<Container<Ts...>> { };

    template <template <typename...> typename List, typename Target, typename... Ts>
    struct find_first_index_of<List<Ts...>, Target>
    {
        constexpr static size_t value = [](){
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
        constexpr static size_t index = [](){
            std::array names { Proj<Ts>::value... };
            return std::ranges::max_element(names) - names.begin();
        }();
    public:
        using type = typename at<List<Ts...>, index>::type;
        constexpr static auto value = Proj<type>::value;
    };

    template <template <typename...> typename List, typename... Ts, template <typename> typename Proj> 
    struct min<List<Ts...>, Proj>
    {
        static_assert(sizeof...(Ts) > 0);
    private:
        constexpr static size_t index = [](){
            std::array names { Proj<Ts>::value... };
            return std::ranges::min_element(names) - names.begin();
        }();
    public:
        using type = typename at<List<Ts...>, index>::type;
        constexpr static auto value = Proj<type>::value;
    };

} // namespace leviathan::metaV2

namespace leviathan::metaV2::detail
{

template <typename... Ts>
struct from
{
private:
    template <auto Tuple, size_t...Idx>
    constexpr static auto to_impl(std::index_sequence<Idx...>)
    {
        return std::index_sequence<Tuple[Idx]...>();
    }

    template <auto Tuple>
    constexpr static auto to_()
    {
        return to_impl<Tuple>(std::make_index_sequence<Tuple.size()>());
    }

    template <auto Tuple, size_t...Idx>
    constexpr static auto select_impl(std::index_sequence<Idx...>)
    {
        using List = type_list<Ts...>;
        return type_list<typename at<List, Tuple[Idx]>::type...>();
    }

    template <auto Tuple>
    constexpr static auto select_()
    {
        return select_impl<Tuple>(std::make_index_sequence<Tuple.size()>());
    }

public:

    template <auto Array>
    struct to
    {
        constexpr static auto array2index = to_<Array>();
        using index_type = decltype(array2index);
    };

    template <auto Array>
    struct select
    {
        constexpr static auto array2index = select_<Array>();
        using index_type = decltype(array2index);
    };
};

}  // namespace detail


namespace leviathan::metaV2 
{

template <template <typename...> typename List, typename... Ts> 
struct reverse<List<Ts...>> 
{
private:
    constexpr static auto indices = [](){
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
//     constexpr static auto indices = [](){
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

} // namespace leviathan::metaV2 