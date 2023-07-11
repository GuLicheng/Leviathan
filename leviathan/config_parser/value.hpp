#pragma once

// #include <leviathan/string/string_extend.hpp>
#include <leviathan/config_parser/optional.hpp>

#include <variant>
#include <memory>
#include <string_view>

namespace leviathan::config
{
    /**
     * @brief This value is used for storing the string of parser.
     * 
     * @param T The storage type of parser, maybe string or string_view.
    */
    template <typename T>
    struct basic_item : optional<T>
    {
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>, "T should not be reference.");

        using optional<T>::optional;

        template <typename U>
        constexpr optional<U> cast() const
        {
            throw 0;
            // if (!*this)
            //     return nullopt;
            // return leviathan::string::lexical_cast<U>(**this);
        }
    };

    using item = basic_item<std::string_view>;

    template <typename Fn, typename... Ts>
    class value_base
    {
    protected:
    
        template <typename T>
        struct mapped
        {
            using type = std::invoke_result_t<Fn, T>;

            static_assert(std::is_same_v<type, std::remove_cvref_t<type>>);
        };

        template <typename T>
        static constexpr bool is_mapped = !(std::is_same_v<T, typename mapped<T>::type>);

        template <typename T>
        struct declaration
        {
            constexpr static bool value = (false || ... || std::is_same_v<T, Ts>);
        };

        template <typename T>
        struct storage
        {
            constexpr static bool value = (false || ... || std::is_same_v<T, typename mapped<Ts>::type>);
        };

        using value_type = std::variant<typename mapped<Ts>::type...>;

        value_type m_data;

    public:

        template <typename Arg>
            requires (declaration<Arg>::value)
        constexpr value_base(Arg arg) : m_data(Fn()(std::move(arg))) { }

        template <typename T>
        constexpr bool is() const
        {
            using U = typename mapped<T>::type;
            return std::holds_alternative<U>(m_data); 
        }
    };

    template <size_t N>
    struct to_unique_ptr_if_large_than
    {
        template <typename T>
        constexpr auto operator()(T&& t) const
        {
            using U = std::remove_cvref_t<T>;
            if constexpr (sizeof(T) > N)
            {
                return std::make_unique<U>((T&&)t);
            }
            else
            {
                return t;
            }
        }
    };

    template <typename T, template <typename...> typename Primary>
    struct is_specialization_of : std::false_type { };

    template <template <typename...> typename Primary, typename... Args>
    struct is_specialization_of<Primary<Args...>, Primary> : std::true_type { };

    template <typename T, template <typename...> typename Primary>
    inline constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;

}


