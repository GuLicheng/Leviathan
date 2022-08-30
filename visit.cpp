#include <iostream>
#include <functional>
#include <utility>
#include <tuple>
#include <variant>

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
template <typename List, typename... Ts> struct flatten_impl;

template <size_t... Idx> 
struct generate_index_sequence_list_by_index_sequence<std::index_sequence<Idx...>>
    : std::type_identity<type_list<std::index_sequence<Idx>...>> { };

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

// https://pbackus.github.io/blog/beating-stdvisit-without-really-trying.html
// we use index_sequence, complier is able to inline the individual visitor functions(maybe...)
template <size_t I, size_t... Idx, typename... IndexSeq, typename Visitor, typename... Vs>
constexpr auto DoVisitImpl(type_list<IndexSeq...> _, std::index_sequence<Idx...>, Visitor&& visitor, Vs&&... vs)
{
    const std::array indices = { vs.index()... };
    constexpr std::array cur_indices = { Idx... };
    
    if constexpr (I + 1 == sizeof...(IndexSeq))
        if (indices == cur_indices)
            return std::invoke((Visitor&&) visitor, std::get<Idx>((Vs&&)vs) ...);
        else
            throw std::bad_variant_access{}; // variant.index() may equal to std::variant::npos
    else
    {
        if (indices == cur_indices)
            // use follow version can generate less asm code.
            // return std::invoke((Visitor&&) visitor, std::__detail::__variant::__get<Idx>((Vs&&)vs) ...);
            return std::invoke((Visitor&&) visitor, std::get<Idx>((Vs&&)vs) ...);
        else
            return DoVisitImpl<I + 1>(_, std::tuple_element_t<I + 1, std::tuple<IndexSeq...>>(), (Visitor&&) visitor, (Vs&&)vs...);
    }
}

template <typename... IndexSeq, typename Visitor, typename... Vs>
constexpr auto DoVisit(type_list<IndexSeq...> indices, Visitor&& visitor, Vs&&... vs)
{
    return DoVisitImpl<0>(indices, std::tuple_element_t<0, std::tuple<IndexSeq...>>(), (Visitor&&) visitor, (Vs&&) vs...);
}

template <typename Visitor, typename V1, typename... Vs>
constexpr auto Visit(Visitor&& visitor, V1&& v1, Vs&&... vs)
{
    constexpr static auto first_size = std::variant_size_v<std::remove_cvref_t<V1>>;
    using first_index_seq = decltype(std::make_index_sequence<first_size>());
    using first_list = typename generate_index_sequence_list_by_index_sequence<first_index_seq>::type;
    using T = typename merge<first_list, std::make_index_sequence<std::variant_size_v<std::remove_cvref_t<Vs>>>...>::type;
    return DoVisit(T{}, (Visitor&&) visitor, (V1&&) v1, (Vs&&) vs...);
}


// tuple_cat
// FIXME: 
// 1. rvalue_reference is not permitted
// 2. automatically unfold tuple-like type, but only one level, why not recursive ?
template <size_t I, typename IndexSeq> struct generate_indices_for_one_tuple;

template <size_t I, size_t... Idx> 
struct generate_indices_for_one_tuple<I, std::index_sequence<Idx...>>
    : std::type_identity<type_list<std::index_sequence<I, Idx>...>> { };

template <typename IndexSeq, typename Tuples> struct generate_indices_for_tuples;

// (0, 1)  { (0, 1) (0, 2) }
template <size_t... Idx, typename... IndexSeq> 
struct generate_indices_for_tuples<std::index_sequence<Idx...>, type_list<IndexSeq...>>
    : std::type_identity<type_list<typename generate_indices_for_one_tuple<Idx, IndexSeq>::type...>> { };


template <typename... IndexSeq, typename Tuple>
auto TupleCatImpl(type_list<IndexSeq...>, Tuple&& tuple)
{
    auto get_element = [&]<size_t OuterIndex, size_t InnerIndex>(std::index_sequence<OuterIndex, InnerIndex>) {
        return std::get<InnerIndex>(std::get<OuterIndex>(tuple));
    };
    return std::make_tuple(get_element(IndexSeq{})...);
}

template <typename... Tuples>
auto TupleCat(Tuples&&... tuples)
{
    using T1 = decltype(std::make_index_sequence<sizeof...(Tuples)>());
    using T2 = type_list<decltype(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuples>>>())...>;
    using T3 = typename generate_indices_for_tuples<T1, T2>::type;
    using T4 = typename flatten<type_list, T3>::type;
    return TupleCatImpl(T4{}, std::forward_as_tuple((Tuples&&)tuples...));
}

#include <iostream>
#include <lv_cpp/meta/template_info.hpp>

void test_variant_visit()
{
    std::variant<int, double> v1 = 1;
    std::variant<int, std::string> v2 = std::string("Hello");
    auto f = [](auto x, auto y) -> int {
        if constexpr (std::is_same_v<decltype(y), std::string>)
            return x + y.size();
        else
            return x + y;
    };

    auto result = Visit(f, v1, v2);
    std::cout << result << '\n';
}

void test_tuple_cat()
{
    auto ret = TupleCat(std::make_tuple(1, 2.0), std::make_tuple(true, std::string("Hello")));
    PrintTypeCategory(ret);
}

int main()
{
    // test_tuple_cat();
    int a = 1;

    std::tuple<int, int&, const int> t{ 1, a, 0 };
    std::tuple<int> t1{ 0 };
    auto ret = std::tuple_cat(t, t1, std::make_tuple(1, std::make_tuple(1, 2, 3)));
    PrintTypeCategory(ret);
}




