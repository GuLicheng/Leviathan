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
struct default_initializer
{
    constexpr auto operator()() const { return T{}; }
};

inline constexpr auto required_initializer = []{};

///////////////////////////////////////////////////////
//   Return type of arg_t for init named_tuple element
///////////////////////////////////////////////////////
template <basic_fixed_string Tag, typename T>
struct tag_value 
{
    constexpr static auto tag() { return Tag; }
    using value_type = T;
    T value; 
};

///////////////////////////////////////
//   Helper class for named argument
///////////////////////////////////////
template <basic_fixed_string Tag>
struct arg_t
{
    template <typename T>
    constexpr auto operator=(T t) const 
    { return tag_value<Tag, T>{std::move(t)}; }
};

template <basic_fixed_string Tag>
inline constexpr auto arg = arg_t<Tag>{ };

template <basic_fixed_string S> 
constexpr arg_t<S> operator ""_arg() { return {}; }

///////////////////////////////////////
//   named_tuple member 
///////////////////////////////////////
template <basic_fixed_string Tag, typename T, auto Initializer = default_initializer<T>()>
struct field
{
    using value_type = T;
    constexpr static auto tag_value = Tag;
    constexpr static auto initializer_value() { return Initializer(); }
};

template <basic_fixed_string... FixedStrings>
struct tag_list_t 
{
    template <basic_fixed_string Tag>
    constexpr static auto value = basic_fixed_string_searcher<Tag, FixedStrings...>::value;
};

template <typename R, typename... Fields, typename... TagValues>
constexpr auto adjust_parameters(TagValues... tvs)
{
    constexpr auto size = sizeof...(TagValues);
    auto params = std::forward_as_tuple(tvs...); 
    auto do_search = [&]<typename Field>() {

        constexpr auto index = basic_fixed_string_searcher<Field::tag_value, tvs.tag() ...>::value;
        // std::cout << "Field is " << Field::tag_value.sv() << " and index = " << index << '\n';
        if constexpr (index == size) 
        {
            // not exist
            return Field::initializer_value();
        }
        else 
        {
            return std::move(std::get<index>(params).value);
        }
    };
    return R{ do_search.template operator()<Fields>()... };
}

template <typename... Fields>
struct named_tuple
{
    using tuple_type = std::tuple<typename Fields::value_type...>;

    constexpr static tag_list_t<Fields::tag_value...> tag_list{ };

    template <typename... TagValues>
    constexpr named_tuple(TagValues... tvs)
        : val(adjust_parameters<tuple_type, Fields...>(std::move(tvs)...)) { }

    template <typename CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const named_tuple& rhs)
    {   
        os << "{\n";
        ((std::cout << '\t' << Fields::tag_value << ": " << rhs.get_with<Fields::tag_value>() << '\n'), ...);
        return os << '}';
    }

    template <basic_fixed_string Tag>
    auto& get_with() 
    {
        // constexpr auto index = basic_fixed_string_searcher<Tag, Fields::tag_value...>::value;
        constexpr auto index = tag_list.template value<Tag>;
        return std::get<index>(val);
    }

    template <basic_fixed_string Tag>
    auto& get_with() const
    {
        constexpr auto index = tag_list.template value<Tag>;
        return std::get<index>(val);
    }

    template <size_t N>
    auto& get_with() { return std::get<N>(val); }

    template <size_t N>
    auto& get_with() const { return std::get<N>(val); }

private:
    tuple_type val; // store values
};

namespace std
{
    using ::named_tuple;

    template <typename... Ts>
    struct tuple_size<named_tuple<Ts...>>
        : integral_constant<size_t, sizeof...(Ts)> { };

    template <size_t N, typename... Ts>
    struct tuple_element<N, named_tuple<Ts...>> 
        : tuple_element<N, typename named_tuple<Ts...>::tuple_type> { };

    template<size_t I, class... Types>
    constexpr tuple_element_t<I, named_tuple<Types...>>&
    get(named_tuple<Types...>& t) noexcept
    {
        return t.template get_with<I>();
    }

    template<size_t I, class... Types>
    constexpr tuple_element_t<I, named_tuple<Types...>>&&
    get(named_tuple<Types...>&& t) noexcept
    {
        return std::move(t.template get_with<I>());
    }

    template<size_t I, class... Types>
    constexpr const tuple_element_t<I, named_tuple<Types...>>&
    get(const named_tuple<Types...>& t) noexcept
    {
        return t.template get_with<I>();
    }

    template<size_t I, class... Types>
    constexpr const tuple_element_t<I, named_tuple<Types...>>&& 
    get(const named_tuple<Types...>&& t) noexcept
    {
        return std::move(t.template get_with<I>());
    }
}
