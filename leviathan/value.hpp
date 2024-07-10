#pragma once

#include <type_traits>
#include <variant>
#include <memory>

namespace leviathan
{
    
/**
 * @brief
 * 
 * @param Fn Store pointer of type T with large object and value with small object.
 * @param Ts... Object types.
 */
template <typename Fn, typename... Ts>
class value
{
protected:

    template <typename T>
    struct actual
    {
        using type = typename Fn::template type<T>;

        static_assert(std::is_same_v<type, std::remove_cvref_t<type>>);
    };

    template <typename T>
    struct declaration
    {
        static constexpr bool value = (false || ... || std::is_same_v<T, Ts>);
    };

    using value_type = std::variant<typename actual<Ts>::type...>;

    value_type m_data;

public:

    constexpr value() = default;

    template <typename Arg>
        requires (declaration<Arg>::value)
    constexpr value(Arg arg) : m_data(Fn::from(std::move(arg))) { }

    constexpr value(const value&) = delete;
    constexpr value& operator=(const value&) = delete;
    
    constexpr value(value&&) = default;
    constexpr value& operator=(value&&) = default;

    template <typename T>
    constexpr bool is() const
    {
        using U = typename actual<T>::type;
        return std::holds_alternative<U>(m_data); 
    }

    template <typename T>
    constexpr T* as_ptr() 
    {
        using U = typename actual<T>::type;
        auto ptr = std::get_if<U>(&m_data);
        return Fn::to_address(ptr);
    }

    template <typename T>
    constexpr const T* as_ptr() const
    {
        return const_cast<value&>(*this).as_ptr();
    }

    template <typename T>
    constexpr T& as()
    {
        return *as_ptr<T>();
    }

    template <typename T>
    constexpr T& as() const
    {
        return const_cast<value&>(*this).as<T>();
    }
};

template <typename T, template <typename...> typename Primary>
struct is_specialization_of : std::false_type { };

template <template <typename...> typename Primary, typename... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type { };

template <typename T, template <typename...> typename Primary>
inline constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;

template <size_t N>
struct to_unique_ptr_if_large_than
{
    template <typename U>
    using type = std::conditional_t<(sizeof(U) > N), std::unique_ptr<U>, U>;

    template <typename T>
    static constexpr auto from(T t) 
    {
        if constexpr (sizeof(T) > N)
        {
            return std::make_unique<T>(std::move(t));
        }
        else
        {
            return t;
        }
    }

    template <typename T>
    static constexpr auto to_address(T* t) 
    {
        if constexpr (!is_specialization_of_v<std::remove_cv_t<T>, std::unique_ptr>)
        {
            return t;
        }
        else
        {
            return std::to_address(*t);
        }
    }
};

} // namespace leviathan

