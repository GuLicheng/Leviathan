#include <assert.h>
#include <iostream>
#include <functional>
#include <utility>
#include <tuple>
#include <variant>
#include <iostream>
// #include <leviathan/meta/template_info.hpp>

/*
    For variant<int, double>, variant<int, double, bool>, we generate follow indices:
    { (0, 0), (0, 1), (0, 2), (1, 0), (1, 1), (1, 2) }
    Then use these to match the variant::index
*/

template <typename... Ts> struct type_list { };

template <typename... Ts> struct merge_index_sequence;

// flatten: traits all types in Ts if Ts is a container(template class) and add all types into Container
// for flatten<type_list, tuple<int>, tuple<bool, tuple<int>>> -> type_list<int, bool, int>
template <template <typename...> typename Container, typename... Ts> struct flatten;

// for (0, 1, 2) -> (0), (1), (2)
template <typename T> struct generate_index_sequence_list_by_index_sequence;

template <typename... Ts> struct merge;

// ---------------------impl----------------------------
template <size_t... Idx> 
struct generate_index_sequence_list_by_index_sequence<std::index_sequence<Idx...>>
    : std::type_identity<type_list<std::index_sequence<Idx>...>> { };

template <typename List, typename... Ts> struct flatten_impl;

template <template <typename...> typename Container, typename... Ts>
struct flatten_impl<Container<Ts...>> : std::enable_if<true, Container<Ts...>> { };

template <template <typename...> typename Container, template <typename...> typename TemplateClass, typename... Ts1, typename... Ts2, typename... Ts>
struct flatten_impl<Container<Ts1...>, TemplateClass<Ts2...>, Ts...>
    : flatten_impl<Container<Ts1...>, Ts2..., Ts...> { };

template <template <typename...> typename Container, typename... Ts1, typename T, typename... Ts>
struct flatten_impl<Container<Ts1...>, T, Ts...>
    : flatten_impl<Container<Ts1..., T>, Ts...> { };

template <template <typename...> typename Container, typename... Ts>
struct flatten : flatten_impl<Container<>, Ts...> { };

template <size_t... Idx1, size_t... Idx2> 
struct merge_index_sequence<std::index_sequence<Idx1...>, std::index_sequence<Idx2...>> 
    : std::type_identity<type_list<std::index_sequence<Idx2..., Idx1>...>> { };

template <typename IndexSeq, typename... Ts> 
struct merge_index_sequence<type_list<Ts...>, IndexSeq>
    : std::type_identity<typename flatten<type_list, type_list<typename merge_index_sequence<IndexSeq, Ts>::type...>>::type> { };

template <typename T1> struct merge<T1> : std::type_identity<T1> { };

template <typename T1, typename T2>
struct merge<T1, T2> : std::type_identity<typename merge_index_sequence<T1, T2>::type> { };

template <typename T1, typename T2, typename T3, typename... Ts> 
struct merge<T1, T2, T3, Ts...> : merge<typename merge<T1, T2>::type, T3, Ts...> { }; 

template <size_t I, typename Variant>
struct check_variant_return
{
    using type = std::conditional_t<std::is_lvalue_reference_v<Variant>,
        std::add_lvalue_reference_t<std::variant_alternative_t<I, std::remove_reference_t<Variant>>>,
        std::add_rvalue_reference_t<std::variant_alternative_t<I, std::remove_reference_t<Variant>>>>;
};

template <size_t I, size_t... Idx, typename... IndexSeq, typename Visitor, typename... Vs>
constexpr auto DoVisitImpl(type_list<IndexSeq...> _, std::index_sequence<Idx...>, Visitor&& visitor, Vs&&... vs)
{
    const std::array indices = { vs.index()... };
    constexpr std::array cur_indices = { Idx... };
        
    if constexpr (I + 1 == sizeof...(IndexSeq))
        return std::invoke((Visitor&&) visitor, (typename check_variant_return<Idx, Vs>::type)*std::get_if<Idx>(std::addressof(vs)) ...);
    else
    {
        if (indices == cur_indices) 
            return std::invoke((Visitor&&) visitor, (typename check_variant_return<Idx, Vs>::type)*std::get_if<Idx>(std::addressof(vs)) ...);
        else
            return DoVisitImpl<I + 1>(_, std::tuple_element_t<I + 1, std::tuple<IndexSeq...>>(), (Visitor&&) visitor, (Vs&&)vs...);
    }
}

template <typename... IndexSeq, typename Visitor, typename... Vs>
constexpr auto DoVisit(type_list<IndexSeq...> indices, Visitor&& visitor, Vs&&... vs)
{
    if ((std::forward<Vs>(vs).valueless_by_exception() || ...))
        throw std::bad_variant_access{};

    return DoVisitImpl<0>(indices, std::tuple_element_t<0, std::tuple<IndexSeq...>>(), (Visitor&&) visitor, (Vs&&) vs...);
}

template <typename Visitor, typename V1, typename... Vs>
constexpr auto VisitMulti(Visitor&& visitor, V1&& v1, Vs&&... vs)
{
    constexpr auto first_size = std::variant_size_v<std::remove_cvref_t<V1>>;
    using first_index_seq = decltype(std::make_index_sequence<first_size>());
    using first_list = typename generate_index_sequence_list_by_index_sequence<first_index_seq>::type;
    using T = typename merge<first_list, std::make_index_sequence<std::variant_size_v<std::remove_cvref_t<Vs>>>...>::type;
    return DoVisit(T{}, (Visitor&&) visitor, (V1&&) v1, (Vs&&) vs...);
}

template <typename Visitor, typename... Vs>
constexpr auto Visit(Visitor&& visitor, Vs&&... vs)
{
    if constexpr (sizeof...(Vs) == 0)
        return ((Visitor&&) visitor)();
    else
        return VisitMulti((Visitor&&) visitor, (Vs&&) vs...);
}

// Source: https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct T0 {};
struct T1 {};


using example_variant = std::variant<T0, T1>;
// https://pbackus.github.io/blog/beating-stdvisit-without-really-trying.html
// GCC 12.1 use `if constexpr` to optimize the case of sizeof...(Vs) == 1, so
// we use two arguments to test whether compiler is able to inline the individual visitor functions.
int do_visit(example_variant v1, example_variant v2)
{
	return std::visit(overloaded {
	// return Visit(overloaded {
		[](T0 , T0) { return 3; },
		[](T1,  T0) { return 5; },
		[](T0,  T1) { return 8; },
		[](T1,  T1) { return 13; },
	}, v1, v2);
}

// void test_variant_visit()
// {
//     const std::variant<int, double> v1 = 1;
//     std::variant<int, std::string> v2 = std::string("Hello");
//     auto f = [](auto x, auto y) -> int {
//         if constexpr (std::is_same_v<decltype(y), std::string>)
//             return x + y.size();
//         else
//             return x + y;
//     };

//     auto result = Visit(f, v1, std::move(v2));

//     std::cout << result << '\n';
// }



int main()
{
    // test_variant_visit();
}




