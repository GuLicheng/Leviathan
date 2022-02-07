#include <tuple>
#include <lv_cpp/utils/struct.hpp>
#include <string_view>
#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/meta/concepts.hpp>
#include <lv_cpp/meta/type_list.hpp>
#include <type_traits>
#include <string>
#include <iostream>
#include <stdint.h>
#include <algorithm>
#include <ranges>
#include <compare>
#include <optional>
#include <variant>

///////////////////////////////////////
//           FixedString
///////////////////////////////////////
template <size_t N>
struct fixed_string
{
    constexpr fixed_string(const char (&foo)[N + 1]) 
    {
        std::copy_n(foo, N + 1, data);
    }
    constexpr auto operator<=>(const fixed_string&) const = default;

    constexpr std::string_view sv() const { return {data, data + N + 1}; }

    template <size_t K>
    constexpr auto operator==(const fixed_string<K>& rhs) const 
    { 
        if constexpr (N == K)
            return std::ranges::equal(data, rhs.data);
        else
            return false;
    }

    template <size_t K>
    constexpr auto operator!=(const fixed_string<K>& rhs) const 
    { return !this->operator==(rhs); }

    char data[N + 1];
};

template <size_t N>
fixed_string(const char (&str)[N]) -> fixed_string<N - 1>; // char[N] is different from const char* with n charactors

// helper meta
template <size_t Cur, auto TargetFixedString, auto... FixedStrings>
struct index_of_impl;

template <size_t Cur, auto TargetFixedString>
struct index_of_impl<Cur, TargetFixedString> 
{
    constexpr static auto value = static_cast<size_t>(-1);
};

template <size_t Cur, auto TargetFixedString, auto FixedString1, auto... FixedStrings>
struct index_of_impl<Cur, TargetFixedString, FixedString1, FixedStrings...> 
{
    constexpr static auto value = 
        TargetFixedString == FixedString1 ? 
            Cur :
            index_of_impl<Cur + 1, TargetFixedString, FixedStrings...>::value;
};

template <auto TargetFixedString, auto... FixedStrings>
struct find_first_fixed_string : index_of_impl<0, TargetFixedString, FixedStrings...> { };


///////////////////////////////////////
//           Initializer
///////////////////////////////////////
template <typename T>
struct default_init
{
    constexpr auto operator()() const { return T{}; }
};

inline constexpr auto required = []{};

template <fixed_string Tag, typename T>
struct tag_value 
{
    constexpr static auto tag() { return Tag; }
    using value_type = T;

    template <typename U>
    constexpr tag_value(const tag_value<Tag, U>& u) : value{u.value} { }

    template <typename U>
    constexpr tag_value(tag_value<Tag, U>&& u) : value{std::move(u.value)} { }

    constexpr tag_value(const T& t) : value{t} { }
    constexpr tag_value(T&& t) noexcept : value{std::move(t)} { }
    constexpr tag_value(const tag_value&) = default;
    constexpr tag_value(tag_value&&) noexcept = default;

    T value; 
};

template <fixed_string Tag>
struct arg_type
{
    template <typename T>
    constexpr auto operator=(T t) const 
    { return tag_value<Tag, T>{std::move(t)}; }
};

template <fixed_string Tag>
inline constexpr auto arg = arg_type<Tag>{ };

template <typename... TagValues>
struct parameters : TagValues... 
{
    parameters(TagValues... vs) : TagValues(std::move(vs))... { } // clang need explicit declearation
    // parameters(parameters&&) = default;
    // parameters(const parameters&) = default;
};

template <typename... Ts> parameters(Ts...) -> parameters<Ts...>;

template <typename... Fields>
struct named_tuple_impl : Fields...
{
    template <typename Parameters>
    constexpr named_tuple_impl(Parameters param)
        : Fields(std::move(param))... { }

};

template <fixed_string Tag, typename T, auto Initer = default_init<T>()>
struct field
{
    constexpr static auto tag() { return Tag; }
    constexpr static auto init() { return Initer; }
    
    using value_type = T;

    template <typename U>
    constexpr explicit field(tag_value<Tag, U> tv) : value(std::move(tv.value)) { }

    // template <typename U>
    // constexpr explicit field(U&& u) : value((U&&)u) { }
    // field(const field&) = default;
    // field(field&&) = default;
    constexpr field() : value(Initer()) { }

    T value;
};

template <typename... Fields, typename... TagValues>
constexpr auto create_parameters(TagValues... tvs)
{
    auto params = std::forward_as_tuple(tvs...);

    // std::cout << "Tags is :"; 
    // (std::cout << ... << tvs.tag().sv());
    // std::cout << '\n';
    // std::cout << "Size is :" << size << '\n'; 

   // Find One Field
    auto do_search = [&]<typename Field>() -> decltype(auto) {
        constexpr auto FieldString = Field::tag();
        constexpr auto index = find_first_fixed_string<FieldString, tvs.tag() ...>::value;
        
        // std::cout << "Field is " << FieldString.sv() << " and index = " << index << '\n';
        
        if constexpr (index == size_t(-1)) 
        {
            // not exist
            return tag_value<FieldString, typename Field::value_type>(Field::init()());
        }
        else 
        {
            return std::move(std::get<index>(params));
        }
    };
    // (do_search.template operator()<Fields>(), ...);
    // ((std::cout << Fields::tag().sv()), ...);
    return parameters<tag_value<Fields::tag(), typename Fields::value_type>...>
        (do_search.template operator()<Fields>()...);
}

template <fixed_string Tag, typename T, auto Init>
decltype(auto) get_impl(field<Tag, T, Init>& v)
{
    return (v.value);
}

template <fixed_string Tag, typename T, auto Init>
decltype(auto) get_impl(const field<Tag, T, Init>& v)
{
    return (v.value);
}



template <typename... Fields>
struct named_tuple : named_tuple_impl<Fields...>
{
    using base = named_tuple_impl<Fields...>;
    template <typename... TagValues>
    constexpr named_tuple(TagValues... tvs)
        // : base(parameters(std::move(tvs)...)) { }
        : base(create_parameters<Fields...>(std::move(tvs)...)) { }

    constexpr named_tuple(parameters<tag_value<Fields::tag(), typename Fields::value_type>...> params)
        : base{std::move(params)} { }

    template <size_t N>
    auto& get_field() const
    {
        using convert_type = std::tuple_element_t<N, std::tuple<Fields...>>;
        return static_cast<const convert_type&>(*this).value;
    }

    template <size_t N>
    auto& get_field() 
    {
        using convert_type = std::tuple_element_t<N, std::tuple<Fields...>>;
        return static_cast<convert_type&>(*this).value;
    }

    template <fixed_string Tag>
    decltype(auto) get_field() 
    {
        return get_impl<Tag>(*this);
    }

    template <fixed_string Tag>
    decltype(auto) get_field() const
    {
        return get_impl<Tag>(*this);
    }


};

namespace std
{

    template <typename... Ts>
    struct tuple_size<named_tuple<Ts...>>
        : integral_constant<size_t, sizeof...(Ts)> { };

    template <size_t N, typename... Ts>
    struct tuple_element<N, named_tuple<Ts...>> 
        : tuple_element<N, tuple<typename Ts::value_type...>> { };

    template<size_t I, class... Types>
    constexpr tuple_element_t<I, named_tuple<Types...>>&
    get(named_tuple<Types...>& t) noexcept
    {
        return t.template get_field<I>();
    }

    template<size_t I, class... Types>
    constexpr tuple_element_t<I, named_tuple<Types...>>&&
    get(named_tuple<Types...>&& t) noexcept
    {
        return std::move(t.template get_field<I>());
    }

    template<size_t I, class... Types>
    constexpr const tuple_element_t<I, named_tuple<Types...>>&
    get(const named_tuple<Types...>& t) noexcept
    {
        return t.template get_field<I>();
    }

    template<size_t I, class... Types>
    constexpr const tuple_element_t<I, named_tuple<Types...>>&& 
    get(const named_tuple<Types...>&& t) noexcept
    {
        return std::move(t.template get_field<I>());
    }

}

using Id_field = field<"id", int>;
using Sex_field = field<"sex", bool>;
using Name_field = field<"name", std::string, []{ return "John"; }>;

using person = named_tuple<
        field<"id", int, required>,
        field<"sex", bool>,
        field<"name", std::string, []{ return "John"; }>
    >;

void print_person(const person& p)
{
    std::cout << "Id = " << std::get<0>(p) << " Sex = " << std::get<1>(p) << " Name = " << std::get<2>(p) << '\n';
}

constexpr fixed_string s1 = "id";
constexpr fixed_string s2 = "sex";
constexpr fixed_string s3 = "name";

static_assert(s1 == s1);
static_assert(s1 != s2);
static_assert(s1 != s3);
static_assert(leviathan::meta::tuple_like<person>);

void test1()
{
    person p1 { arg<"id"> = 0, arg<"name"> = "Alice", arg<"sex"> = std::optional<bool>(true) };
    person p2 { arg<"id"> = 1 };

    print_person(p1);
    print_person(p2);
    std::cout << p1.get_field<"name">() << '\n';
}

void test2()
{
    using T = named_tuple<field<"int", Int32>, field<"name", double>>;
    // T t{ arg<"int"> = 1 };
    T t;
    std::cout << Int32::move_constructor << '\n';
    std::cout << Int32::copy_constructor << '\n';
    std::cout << Int32::total_construct() << '\n';
}

int main()
{
    test1();
    test2();
}





