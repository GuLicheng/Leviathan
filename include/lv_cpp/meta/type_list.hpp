#ifndef __TYPE_LIST_HPP__
#define __TYPE_LIST_HPP__

#include <type_traits>
#include <tuple>  //  for tuple
#include <cstddef>  // for size_t

// Here are some options for type list
namespace leviathan::meta
{

// using empty_list = std::tuple<>;

// -------------to_type_list--------------
// put types into type list
template <typename... Types>
struct to_type_list : std::enable_if<true, std::tuple<Types...>> {};

template <template <typename...> typename TemplateClass, typename... Types>
struct to_type_list<TemplateClass<Types...>>
    : std::enable_if<true, std::tuple<Types...>> {};


// -------traits_parameters_from_type_list-------
// traits types from Container into TemplateClass
template <template <typename...> typename T, typename...>
struct traits_parameters_from_type_list;

template <template <typename...> typename TemplateClass, 
        template <typename...> typename Container, typename... Types>
struct traits_parameters_from_type_list<TemplateClass, Container<Types...>> 
    : std::enable_if<true, TemplateClass<Types...>> { };




namespace detail
{
template <typename... Lists>
struct concat_impl;

template <template <typename...> typename Container1, template <typename...> typename Container2, 
                                typename... Ts1, typename... Ts2>
struct concat_impl<Container1<Ts1...>, Container2<Ts2...>>
    : std::enable_if<true, Container1<Ts1..., Ts2...>> { };


template <template <typename...> typename Container1, template <typename...> typename Container2, 
                                typename... Ts1, typename... Ts2, typename... Containers>
struct concat_impl<Container1<Ts1...>, Container2<Ts2...>, Containers...>
    : concat_impl<Container1<Ts1..., Ts2...>, Containers...> { };

} // namespace detail

// -------------------------concat-------------------------------
// traits all types in each typelist into Container
template <template <typename...> typename Container, typename... Containers>
struct concat 
    : std::enable_if <true,
            typename ::leviathan::meta::traits_parameters_from_type_list
            <
                Container, typename detail::concat_impl<std::tuple<>, Containers...>::type    
            >::type
        > 
        { }; 


namespace detail
{
template <typename List, typename... Ts>
struct flatten_impl;

template <template <typename...> typename Container, typename... Ts>
struct flatten_impl<Container<Ts...>> : std::enable_if<true, Container<Ts...>> { };

template <template <typename...> typename Container, template <typename...> typename TemplateClass, typename... Ts1, typename... Ts2, typename... Ts>
struct flatten_impl<Container<Ts1...>, TemplateClass<Ts2...>, Ts...>
    : flatten_impl<Container<Ts1...>, Ts2..., Ts...> { };

template <template <typename...> typename Container, typename... Ts1, typename T, typename... Ts>
struct flatten_impl<Container<Ts1...>, T, Ts...>
    : flatten_impl<Container<Ts1..., T>, Ts...> { };

} // namespace detail

// --------------------flatten------------------------
// traits all types in Ts if Ts is a container(template class) and add all types into Container
template <template <typename...> typename Container, typename... Ts>
struct flatten : detail::flatten_impl<Container<>, Ts...> { };
/*
    using T = variant<pair<int, tuple<int, double, bool, pair<int, tuple<>>>>>;
    you can view T as ((int, (int, double, bool), (int, ())))
    after remove all template class and traits types of them:
    flatten<T>::type -> (int, int, double, bool, int) 
*/


// --------------size-----------------
// return the size of type list
template <typename... Ts>
struct size : size<std::tuple<Ts...>> { };


template <typename... Types>
struct size<std::tuple<Types...>>
    : std::integral_constant<size_t, sizeof...(Types)> {};


// ------------------front---------------------------
// return the first type of type list
template <typename...   Ts>
struct front : front<std::tuple<Ts...>> { };

template <typename List, typename... Types>
struct front<std::tuple<List, Types...>> : std::enable_if<true, List> {};


// ------------------type_empty------------------------------
// return whether the type list is empty
template <typename... Ts>
struct empty : empty<std::tuple<Ts...>> { };

template <typename... Types>
struct empty<std::tuple<Types...>>
    : std::conditional_t<(sizeof...(Types) == 0), std::true_type,
                         std::false_type> {};


// ------------------push_front-----------------------
// add some types in the front type list
template <typename List, typename... Ts>
struct push_front : push_front<List, std::tuple<Ts...>> { };

template <typename... Ts, typename... Ts1>
struct push_front<std::tuple<Ts...>, std::tuple<Ts1...>> 
    : std::enable_if<true, std::tuple<Ts1..., Ts...>> { };


// -------------pop_front----------------
// remove the first type of type list
template <typename... Ts>
struct pop_front : pop_front<std::tuple<Ts...>> { };

template <typename T, typename... Types>
struct pop_front<std::tuple<T, Types...>>
    : std::enable_if<true, std::tuple<Types...>> {};

// ------------------index_of---------------------
// get the Index type of Ts
template <size_t Index, typename... Ts>
struct index_of : index_of<Index, std::tuple<Ts...>> { };


template <typename T, typename... Types>
struct index_of<0, std::tuple<T, Types...>> : 
    std::enable_if<true, T> {};

template <size_t Index, typename T1, typename... Types>
struct index_of<Index, std::tuple<T1, Types...>>
    : index_of<Index - 1, std::tuple<Types...>> {};

// -----------------back----------------
// return the last type of type list
template <typename... Ts>
struct back : back<std::tuple<Ts...>> { };

template <typename T>
struct back<std::tuple<T>> : std::enable_if<true, T> {};

template <typename T, typename... Types>
struct back<std::tuple<T, Types...>> : back<std::tuple<Types...>> {};

namespace detail 
{
// another way in c++17 to implement back with O(1) instantiation depth
template <typename... Types>
struct select_last 
{
    using type = typename decltype((std::enable_if<true, Types>{}, ...))::type;
};

}  // namespace detail

// ----------------push_back-------------------
// add type at the last position 
template <typename List, typename... Ts>
struct push_back : push_back<List, std::tuple<Ts...>> { };

template <typename... Ts, typename... Ts1>
struct push_back<std::tuple<Ts...>, std::tuple<Ts1...>>
    : std::enable_if<true, std::tuple<Ts..., Ts1...>> { };


// -------------------------pop_back-------------------------
// remove the last type in type list
template <typename... Ts>
struct pop_back : pop_back<std::tuple<Ts...>> { };

namespace detail 
{
template <typename List1, typename List2>
struct pop_back_helper;

template <typename... Types1, typename... Types2, typename T1, typename T2>
struct pop_back_helper<std::tuple<Types1...>, std::tuple<T1, T2, Types2...>>
    : pop_back_helper<std::tuple<Types1..., T1>, std::tuple<T2, Types2...>> {};

template <typename... Types, typename T>
struct pop_back_helper<std::tuple<Types...>, std::tuple<T>>
    : std::enable_if<true, std::tuple<Types...>> {};

}  // namespace detail

template <typename... Types>
struct pop_back<std::tuple<Types...>>
    : detail::pop_back_helper<std::tuple<>, std::tuple<Types...>> {};




}  // namespace leviathan::meta

// Here are some algorithms based on type list
namespace leviathan::meta
{
// ----------------------max_type-----------------------
// return the max type in type list
template <typename... Ts>
struct max_type : max_type<std::tuple<Ts...>> { };

template <typename T>
struct max_type<std::tuple<T>> : std::enable_if<true, T> {};

template <typename T1, typename T2, typename... Types>
struct max_type<std::tuple<T1, T2, Types...>>
    : std::conditional_t<(sizeof(T1) > sizeof(T2)),
                         max_type<std::tuple<T1, Types...>>,
                         max_type<std::tuple<T2, Types...>>> {};

// ----------------------min_type-----------------------
// return the min type in type list
template <typename... Ts>
struct min_type : min_type<std::tuple<Ts...>> { };

template <typename T>
struct min_type<std::tuple<T>> : std::enable_if<true, T> {};

template <typename T1, typename T2, typename... Types>
struct min_type<std::tuple<T1, T2, Types...>>
    : std::conditional_t<(sizeof(T1) < sizeof(T2)),
                         min_type<std::tuple<T1, Types...>>,
                         min_type<std::tuple<T2, Types...>>> {};



namespace detail 
{
template <typename List1, typename List2>
struct reverse_helper;

template <typename T, typename... Types1, typename... Types2>
struct reverse_helper<std::tuple<T, Types1...>, std::tuple<Types2...>>
    : reverse_helper<std::tuple<Types1...>, std::tuple<T, Types2...>> {};

template <typename... Types>
struct reverse_helper<std::tuple<>, std::tuple<Types...>>
    : std::enable_if<true, std::tuple<Types...>> {};

}  // namespace detail


// ------------reverse---------------
// reverse types in type list
template <typename... Ts>
struct reverse : reverse<std::tuple<Ts...>> { };

template <typename T>
struct reverse<std::tuple<T>> : std::enable_if<true, std::tuple<T>> {};

template <typename T1, typename T2, typename... Types>
struct reverse<std::tuple<T1, T2, Types...>>
    : detail::reverse_helper<std::tuple<T2, Types...>, std::tuple<T1>> {};


// --------------find_first_index_of-----------------
// find the first type in Typeist
template <typename List, typename TargetType>
struct find_first_index_of;

namespace detail 
{
    template <size_t Index, typename List, typename TargetType>
    struct find_first_index_of_helper;

    template <size_t Index, typename T1, typename...Types, typename TargetType>
    struct find_first_index_of_helper<Index,  std::tuple<T1, Types...>, TargetType> 
            : std::conditional_t<std::is_same_v<T1, TargetType>, 
                                 std::integral_constant<size_t, Index>,
                                 find_first_index_of_helper<Index + 1, std::tuple<Types...>, 
                                 TargetType>
                                > { };

    constexpr size_t npos = static_cast<size_t>(-1);
    template <size_t Index, typename TargetType>
    struct find_first_index_of_helper<Index, std::tuple<>, TargetType>
        : std::integral_constant<size_t, npos> { };
}  // namespace detail

template <typename... Types, typename TargetType>
struct find_first_index_of<std::tuple<Types...>, TargetType> 
    : detail::find_first_index_of_helper<0, std::tuple<Types...>, TargetType> { };



// insert sort
namespace detail 
{

template <typename List, typename T, size_t N>
struct find_index_in_list;

template <typename List, typename T, size_t N>
struct insert;

template <typename List1, typename List2, typename T, size_t N>
struct insert_impl;

template <typename List1, typename List2>
struct insert_sort_impl;

template <typename... Ts1, typename T1, typename... Ts2>
struct insert_sort_impl<std::tuple<Ts1...>, std::tuple<T1, Ts2...>>
    : insert_sort_impl<
                    typename insert<std::tuple<Ts1...>, T1, find_index_in_list<std::tuple<Ts1...>, T1, 0>::value>::type, 
                    std::tuple<Ts2...>
                    >
    { };


template <typename... Ts>
struct insert_sort_impl<std::tuple<Ts...>, std::tuple<>>
    : std::enable_if<true, std::tuple<Ts...>>
    { };


template <typename ... Ts1, typename... Ts2, typename T2, typename T, size_t N>
struct insert_impl<std::tuple<Ts1...>, std::tuple<T2, Ts2...>, T, N>
    : std::conditional_t<
                        N == 0,
                        std::enable_if<true, std::tuple<Ts1..., T, T2, Ts2...>>,
                        insert_impl<std::tuple<Ts1..., T2>, std::tuple<Ts2...>, T, N - 1>
                        >
    { };

template <typename ... Ts1, typename T, size_t N>
struct insert_impl<std::tuple<Ts1...>, std::tuple<>, T, N>
    : std::enable_if<true, std::tuple<Ts1..., T>>
    { };

template <typename... Ts, typename T, size_t Idx>
struct insert<std::tuple<Ts...>, T, Idx> 
    : insert_impl<std::tuple<>, std::tuple<Ts...>, T, Idx>
    { };

template <typename T1, typename... Ts, typename T, size_t N>
struct find_index_in_list<std::tuple<T1, Ts...>, T, N>
    : std::conditional_t<
                        sizeof(T1) <= sizeof(T),
                        find_index_in_list<std::tuple<Ts...>, T, N + 1>,
                        std::integral_constant<size_t, N>
                        >
    { };

template <typename T, size_t N>
struct find_index_in_list<std::tuple<>, T, N>
    : std::integral_constant<size_t, N> 
    { };

} // namespace detail

template <typename... Ts>
struct insert_sort : insert_sort<std::tuple<Ts...>> { };

template <typename... Ts>
struct insert_sort<std::tuple<Ts...>> : detail::insert_sort_impl<std::tuple<>, std::tuple<Ts...>> 
    { };

/*
    class foo
    { 
        double x, y; 
    };
    using T1 = std::tuple<>;
    using T2 = std::tuple<int>;
    using T3 = std::tuple<int, double, char, bool, foo, short>;

    using T4 = leviathan::type::insert_sort<T1>::type;
    using T5 = leviathan::type::insert_sort<T2>::type;
    using T6 = leviathan::type::insert_sort<T3>::type;
    static_assert(std::is_same_v<std::tuple<>, T4>);
    static_assert(std::is_same_v<std::tuple<int>, T5>);
    static_assert(std::is_same_v<std::tuple<char, bool, short, int, double, foo>, T6>);
*/

namespace detail
{

template <typename List1, typename List2>
struct unique_impl;

template <typename... Ts1, typename... Ts2, typename T>
struct unique_impl<std::tuple<Ts1...>, std::tuple<T, Ts2...>>
    : std::conditional<(find_first_index_of<std::tuple<Ts1...>, T>::value != npos),
                        typename unique_impl<std::tuple<Ts1...>, std::tuple<Ts2...>>::type,
                        typename unique_impl<std::tuple<Ts1..., T>, std::tuple<Ts2...>>::type> { };

template <typename... Ts>
struct unique_impl<std::tuple<Ts...>, std::tuple<>>
    : std::enable_if<true, std::tuple<Ts...>> { };

} // namespace detail

template <typename... Ts>
struct unique : unique<std::tuple<Ts...>> { };

template <typename... Ts>
struct unique<std::tuple<Ts...>> 
    : detail::unique_impl<std::tuple<>, std::tuple<Ts...>> { };

/*
    using T = std::tuple<int, char, double, char, int, bool, int, double, float>;
    using Res1 = leviathan::type::unique<T>::type;
    using Res2 = leviathan::type::unique<int, char, double, char, int, bool, int, double, float>::type;
    static_assert(std::is_same_v<Res1, Res2>);
*/

/*
    2020/12/14 -- specialize variadic parameters for basic template meta
    constexpr auto size = leviathan::type::size<int, double>::value;
    using F1 = leviathan::type::front<int, double, char>::type;
    using F2 = leviathan::type::pop_front<int, double, char>::type;
    using F3 = leviathan::type::max_type<int, double, char>::type;
    using F4 = leviathan::type::min_type<int, double, char>::type;
    using F5 = leviathan::type::index_of<2, int, double, char>::type;
    using F6 = leviathan::type::reverse<int, double, char>::type;
    using F7 = leviathan::type::back<int, double, char>::type;
    using F8 = leviathan::type::pop_back<int, double, char>::type;
*/

template <template <typename...> typename Func, typename... Args>
struct transform
     : std::enable_if<true, std::tuple<typename Func<Args>::type ...>> { };


template <template <typename...> typename Func, typename... Args>
struct call : std::enable_if<true, typename Func<Args...>::type> { };
// {
//     using type = typename Func<Args...>::type;
// };

} // namespace leviathan::meta

// for function_traits
namespace leviathan::meta
{

namespace detail
{
    
template <bool IsNoThrow, typename R, typename... Args>
struct function_traits_impl
{
    static constexpr auto argc = sizeof...(Args);

    using return_type = R;

    using args = std::tuple<Args...>;

    template <std::size_t N>
    using nth_arg = std::tuple_element_t<N, std::tuple<Args...>>;

    constexpr static bool is_noexcept = IsNoThrow;

};

} // namespace detail

template <typename T>
struct function_traits;
/*
    function x 3
    member_function x 12
    not support overload function
*/

// int(*)(int, int)
template <typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(*)(Args...) noexcept(IsNoThrow)> 
    : detail::function_traits_impl<IsNoThrow, R, Args...> { };

// int(&)(int, int)
template <typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(&)(Args...) noexcept(IsNoThrow)> 
    : detail::function_traits_impl<IsNoThrow, R, Args...> { };

// int(int, int)
template <typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(Args... ) noexcept(IsNoThrow)> 
    : detail::function_traits_impl<IsNoThrow, R, Args...> { };


// no any cv_ref
template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> 
    : detail::function_traits_impl<IsNoThrow, R, Args...>
{
    using class_type = ClassType;
};

// const
template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const noexcept(IsNoThrow)> 
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

// volatile
template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) volatile noexcept(IsNoThrow)> 
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };


// const volatile
template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const volatile noexcept(IsNoThrow)> 
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

// const&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const& noexcept(IsNoThrow)>
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

// &
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) & noexcept(IsNoThrow)>
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

// volatile&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) volatile& noexcept(IsNoThrow)>
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

// const volatile&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const volatile& noexcept(IsNoThrow)>
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

// &&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) && noexcept(IsNoThrow)>
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

// const&&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const&& noexcept(IsNoThrow)>
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

// volatile&&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) volatile&& noexcept(IsNoThrow)>
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

// const volatile&&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const volatile&& noexcept(IsNoThrow)>
    : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };



// specialize for operator()
template <typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};


} // leviathan::meta

// for some utilities
namespace leviathan::meta
{
template <template <typename...> typename TemplateClass, typename... Args>
struct is_instance : std::false_type { };

template <template <typename...> typename TemplateClass, typename... Args>
struct is_instance<TemplateClass, TemplateClass<Args...>> : std::true_type { };

namespace detail
{
    
template <size_t N, typename Res, typename T>
struct repeat_impl
    : repeat_impl<N - 1, typename push_back<Res, T>::type, T> 
{ };

template <typename Res, typename T>
struct repeat_impl<0, Res, T> : std::enable_if<true, Res> { };

} // namespace detail

// repeat T for N and put them into Container
template <size_t N, typename T>
struct repeat : detail::repeat_impl<N, std::tuple<>, T> { };





}  // namespace leviathan meta

#endif // __TYPE_LIST_HPP__