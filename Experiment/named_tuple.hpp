#pragma once

#include <lv_cpp/string/fixed_string.hpp>
#include <stdint.h>
#include <tuple>
#include <string_view>
#include <type_traits>
#include <algorithm>

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

    template <typename U> requires (std::is_constructible_v<T, U>)
    constexpr tag_value(U&& u) : value{(U&&)u} { }

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
    constexpr named_tuple_impl(Parameters&& param)
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

    constexpr field() : value(Initer()) { }

    template <typename CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const field& rhs)
    {
        os << Tag << ": " << rhs.value;
        return os;
    }

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

    template <typename CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const named_tuple& rhs)
    {
        int count = 0;
        os << '(';
        ((count++ == 0 ?
             os << static_cast<const Fields&>(rhs) : 
             os << ' ' << static_cast<const Fields&>(rhs)) , ...);
        return os << ')';
    }

};

namespace std
{
    using ::named_tuple;

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










