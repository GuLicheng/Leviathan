#pragma once

#include <lv_cpp/meta/template_info.hpp> // TypeInfo -> convert type to string
#include <type_traits>
#include <tuple>
#include <algorithm>  // process strings
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

    template <typename Lits, size_t Idx> struct at;

    template <typename Lits, template <typename> typename Proj = size_of> struct max;

    template <typename Lits, template <typename> typename Proj = size_of> struct min;
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


}




