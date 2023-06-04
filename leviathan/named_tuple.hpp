
#include <leviathan/string/fixed_string.hpp>
#include <stdint.h>
#include <tuple>
#include <string_view>
#include <type_traits>
#include <iostream>


namespace leviathan
{
    /**
     * @brief Create instance with default construction.
    */
    template <typename T>
    struct default_initializer
    {
        constexpr static auto operator()() { return T{}; }
    };

    /**
     * @brief: This lambda return incomplete type void and will make
     *  compiler error if field use it as initializer and not provide
     *  argument for constructing.
    */
    inline constexpr auto required_initializer = []{};

    /**
     * @brief Return type of arg_t for init named_tuple element.
    */
    template <basic_fixed_string Tag, typename T>
    struct tag_value 
    {
        constexpr static auto tag() { return Tag; }

        using value_type = T;

        T value; 
    };

    template <typename T>
    struct is_tag_value : std::false_type { };

    template <basic_fixed_string Tag, typename T>
    struct is_tag_value<tag_value<Tag, T>> : std::true_type { };

    /**
     * @brief Helper class for named argument.
    */
    template <basic_fixed_string Tag>
    struct arg_t
    {
        constexpr static auto tag() { return Tag; }

        template <typename T>
        constexpr auto operator=(T t) const 
        { return tag_value<Tag, T>{ .value = std::move(t)}; }

    };

    template <basic_fixed_string Tag>
    inline constexpr auto arg = arg_t<Tag>{ };

    /**
     * @brief Member field of named_tuple.
     * 
     * @param Tag Name of field.
     * @param T Type of field.
     * @param Initializer initialize T when not supplying argument of field.
    */
    template <basic_fixed_string Tag, typename T, auto Initializer = default_initializer<T>()>
    struct field
    {
        using value_type = T;
        constexpr static auto tag_value = Tag;
        constexpr static auto initializer_value() { return Initializer(); }
    };

    namespace detail
    {
        /**
         * @brief Cplusplus requires the initialization order shoule keep up with declaration order.
         *  The function will automatically adjust the order of arguments so that it can satisfy 
         *  the requirement of cplusplus.
         * 
         * @param R Return type. Using auto is OK, but offer R may faster for compiler-time.
        */
        template <typename R, typename... Fields, typename... TagValues>
        constexpr auto adjust_parameters(TagValues... tvs)
        {
            constexpr auto size = sizeof...(TagValues);  
            auto params = std::forward_as_tuple(tvs...); // save params

            auto do_search = [&]<typename Field>() {
                constexpr auto index = fixed_string_list<tvs.tag()...>::template index_of<Field::tag_value>;
                if constexpr (index == size) 
                    return Field::initializer_value(); // not exist
                else 
                    return std::move(std::get<index>(params).value);
            };
            return R(do_search.template operator()<Fields>()...);
        }
    }


    template <typename... Fields>
    class named_tuple
    {
        constexpr static fixed_string_list<Fields::tag_value...> tag_list { };
    public:
        using tuple_type = std::tuple<typename Fields::value_type...>;

        template <size_t N>
        constexpr std::string_view name_of() const
        {
            using T = std::tuple_element_t<N, std::tuple<Fields...>>;
            return T::tag_value.sv();
        }

        template <typename... TagValues>
        constexpr named_tuple(TagValues... tvs)
            : val(detail::adjust_parameters<tuple_type, Fields...>(std::move(tvs)...)) 
        {
            // avoid error name
            static_assert((tag_list.template contains<TagValues::tag()> && ...), "Unknown Tag");
        }
        constexpr named_tuple() = default;
        constexpr named_tuple(const named_tuple&) = default; // FIX ME
        constexpr named_tuple(named_tuple&&) noexcept(true) = default; // FIX ME
        constexpr named_tuple& operator=(const named_tuple&) = default; // FIX ME
        constexpr named_tuple& operator=(named_tuple&&) noexcept(true) = default; // FIX ME

        template <typename CharT>
        friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const named_tuple& rhs)
        {   
            os << '{' << ' ';
            ((std::cout << Fields::tag_value << ": " << rhs.get_with<Fields::tag_value>() << ", "), ...);
            return os << '}';
        }

        template <basic_fixed_string Tag>
        auto& get_with() 
        {
            constexpr auto index = tag_list.template index_of<Tag>;
            return std::get<index>(val);
        }

        template <basic_fixed_string Tag>
        auto& get_with() const
        {
            constexpr auto index = tag_list.template index_of<Tag>;
            return std::get<index>(val);
        }

        template <size_t N>
        auto& get_with() { return std::get<N>(val); }

        template <size_t N>
        auto& get_with() const { return std::get<N>(val); }

        template <typename T>
        constexpr decltype(auto) operator[](T) const
        {
            constexpr auto tag = T::tag();
            return get_with<tag>();
        }

        template <typename T>
        constexpr decltype(auto) operator[](T) 
        {
            constexpr auto tag = T::tag();
            return get_with<tag>();
        }

    private:
    
        tuple_type val; // store values

    };

} // namespace leviathan

namespace std
{
    template <typename... Ts>
    struct tuple_size<leviathan::named_tuple<Ts...>>
        : integral_constant<size_t, sizeof...(Ts)> { };

    template <size_t N, typename... Ts>
    struct tuple_element<N, leviathan::named_tuple<Ts...>> 
        : tuple_element<N, typename leviathan::named_tuple<Ts...>::tuple_type> { };

    template<size_t I, class... Types>
    constexpr tuple_element_t<I, leviathan::named_tuple<Types...>>&
    get(leviathan::named_tuple<Types...>& t) noexcept
    {
        return t.template get_with<I>();
    }

    template<size_t I, class... Types>
    constexpr tuple_element_t<I, leviathan::named_tuple<Types...>>&&
    get(leviathan::named_tuple<Types...>&& t) noexcept
    {
        return std::move(t.template get_with<I>());
    }

    template<size_t I, class... Types>
    constexpr const tuple_element_t<I, leviathan::named_tuple<Types...>>&
    get(const leviathan::named_tuple<Types...>& t) noexcept
    {
        return t.template get_with<I>();
    }

    template<size_t I, class... Types>
    constexpr const tuple_element_t<I, leviathan::named_tuple<Types...>>&& 
    get(const leviathan::named_tuple<Types...>&& t) noexcept
    {
        return std::move(t.template get_with<I>());
    }
}

namespace leviathan::literals
{
    template <basic_fixed_string S> 
    constexpr arg_t<S> operator ""_arg() { return {}; }
}

using namespace leviathan::literals;
