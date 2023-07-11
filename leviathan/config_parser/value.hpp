#pragma once

// #include <leviathan/string/string_extend.hpp>
#include <leviathan/config_parser/optional.hpp>

#include <variant>
#include <memory>
#include <string_view>
#include <string>
#include <type_traits>

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

    template <typename T>
    concept arithmetic = std::is_arithmetic_v<T>;

    template <typename T>
    concept string_viewable = std::is_convertible_v<const T&, std::string_view>;

    template <typename T>
    concept string_view_like = string_viewable<T> && !std::is_convertible_v<const T&, const char*>;

    struct string_hash_key_equal
    {
        using is_transparent = void;

        template <string_viewable Lhs, string_viewable Rhs>
        constexpr bool operator()(const Lhs& l, const Rhs& r) const
        {
            std::string_view sl = static_cast<std::string_view>(l);
            std::string_view sr = static_cast<std::string_view>(r);
            return compare_impl(sl, sr);
        }

        template <string_viewable Str>
        constexpr bool operator()(const Str& s) const
        {
            std::string_view sv = static_cast<std::string_view>(s);
            return hash_impl(sv);
        }

    private:

        constexpr static bool compare_impl(std::string_view lhs, std::string_view rhs)
        { return lhs == rhs; }  

        constexpr static size_t hash_impl(std::string_view sv)
        { return std::hash<std::string_view>()(sv); }
    };
 
    inline constexpr const char* whitespace_delimiters = " \t\n\r";

    constexpr std::string_view ltrim(std::string_view sv, std::string_view delimiters = whitespace_delimiters)
    {
        auto left = sv.find_first_not_of(delimiters);
        return sv.substr(left == sv.npos ? sv.size() : left);
    }

    constexpr std::string_view rtrim(std::string_view sv, std::string_view delimiters = whitespace_delimiters)
    {
        auto right = sv.find_last_not_of(delimiters);
        return sv.substr(0, right + 1);
    }

    constexpr std::string_view trim(std::string_view sv, std::string_view delimiters = whitespace_delimiters)
    { return rtrim(ltrim(sv, delimiters), delimiters); }

    std::string_view trim(const std::string& s, std::string_view delimiters = whitespace_delimiters)
    {
        std::string_view sv = s;
        return trim(sv, delimiters);        
    }

    inline std::string replace(std::string str, std::string_view from, std::string_view to)
    {
        size_t pos = 0;
        while ((pos = str.find(from, pos)) != str.npos)
        {
            str.replace(pos, from.size(), to);
            pos += to.size();
        }
        return str;
    }
}
