#include <lv_cpp/meta/template_info.hpp>
#include <iostream>
#include <functional>
#include <utility>
#include <tuple>

template <typename... Ts> struct type_list { };

template <typename... Ts> struct merge_index_sequence;

template <typename IndexSeq, typename TypeList> struct merge_index_sequence_and_type_list; 

// flatten: traits all types in Ts if Ts is a container(template class) and add all types into Container
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


#include <variant>



template <size_t I, size_t... Idx, typename... IndexSeq, typename Visitor, typename... Vs>
auto DoVisitImpl(type_list<IndexSeq...> _, std::index_sequence<Idx...>, Visitor&& visitor, Vs&&... vs)
{
    const std::array indices = { vs.index()... };
    const std::array cur_indices = { Idx... };
    
    // std::__detail::__variant::__get()
    
    if constexpr (I + 1 == sizeof...(IndexSeq))
        if (indices == cur_indices)
            return std::invoke((Visitor&&) visitor, std::get<Idx>((Vs&&)vs) ...);
        else
            throw std::bad_variant_access{}; // variant.index() may equal to std::variant::npos
    else
    {
        if (indices == cur_indices)
            return std::invoke((Visitor&&) visitor, std::get<Idx>((Vs&&)vs) ...);
            // return std::invoke((Visitor&&) visitor, std::__detail::__variant::__get<Idx>((Vs&&)vs) ...);
        else
            return DoVisitImpl<I + 1>(_, std::tuple_element_t<I + 1, std::tuple<IndexSeq...>>(), (Visitor&&) visitor, (Vs&&)vs...);
    }
}


template <typename... IndexSeq, typename Visitor, typename... Vs>
auto DoVisit(type_list<IndexSeq...> _, Visitor&& visitor, Vs&&... vs)
{
    return DoVisitImpl<0>(_, std::tuple_element_t<0, std::tuple<IndexSeq...>>(), (Visitor&&) visitor, (Vs&&) vs...);
}

template <typename Visitor, typename V1, typename... Vs>
auto Visit(Visitor&& visitor, V1&& v1, Vs&&... vs)
{
    constexpr static auto first_size = std::variant_size_v<std::remove_cvref_t<V1>>;
    using first_index_seq = decltype(std::make_index_sequence<first_size>());
    using first_list = typename generate_index_sequence_list_by_index_sequence<first_index_seq>::type;
    using T = typename merge<first_list, std::make_index_sequence<std::variant_size_v<std::remove_cvref_t<Vs>>>...>::type;
    return DoVisit(T{}, (Visitor&&) visitor, (V1&&) v1, (Vs&&) vs...);
}



#include <variant>

// Source: https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct T0 {};
struct T1 {};
struct T2 {};
struct T3 {};
struct T4 {};
struct T5 {};
struct T6 {};
struct T7 {};
struct T8 {};
struct T9 {};

using example_variant = std::variant<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>;

int do_visit(example_variant v)
{
	return std::visit(overloaded {
		[](T0 val) { return 3; },
		[](T1 val) { return 5; },
		[](T2 val) { return 8; },
		[](T3 val) { return 13; },
		[](T4 val) { return 21; },
		[](T5 val) { return 34; },
		[](T6 val) { return 55; },
		[](T7 val) { return 89; },
		[](T8 val) { return 144; },
		[](T9 val) { return 233; },
	}, v);
}

int main()
{
    std::variant<int, double> v1 = 1;
    std::variant<int, std::string> v2 = std::string("Hello");
    auto f = [](auto x) -> int {
        return x;
    };

    auto result = Visit(f, v1);
    std::cout << result << '\n';

}




