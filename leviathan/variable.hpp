#pragma once

#include <leviathan/extc++/concepts.hpp>
#include <functional>
#include <type_traits>
#include <variant>
#include <memory>

namespace leviathan
{
    
/**
 * @brief
 * 
 * @param Fn Adaptor, convert value to storage type and access object.
 * @param Ts... Object types.
 */
template <typename Fn, typename... Ts>
class variable
{
    static_assert(sizeof...(Ts) > 0);

public:

    template <typename T>
    struct actual
    {
        using type = typename Fn::template type<T>;

        static_assert(std::is_same_v<type, std::remove_cvref_t<type>>);
    };

    struct accessor
    {
        template <typename T>
        constexpr static decltype(auto) operator()(T&& x)
        {
            auto ptr = Fn::to_address(std::addressof(x));
            return std::forward_like<T>(*ptr);
        }
    };

protected:

    template <typename T>
    struct declaration
    {
        static constexpr bool value = (false || ... || std::is_same_v<T, Ts>);
    };

    using value_type = std::variant<typename actual<Ts>::type...>;

    value_type m_data;

public:

    constexpr variable() = default;

    template <typename Arg>
        requires (declaration<Arg>::value)
    constexpr variable(Arg arg) : m_data(Fn::from_value(std::move(arg))) { }

    constexpr variable(const variable&) = delete;
    constexpr variable& operator=(const variable&) = delete;
    
    constexpr variable(variable&&) = default;
    constexpr variable& operator=(variable&&) = default;

    // Compare each type directly may be incorrect. For example, if lhs 
    // contains 'int' and rhs contains 'double', the lhs('int') can directly
    // compared with rhs('double'), but they hold different types.   
    // constexpr bool operator==(const variable& rhs) const;

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
        return const_cast<variable&>(*this).as_ptr<T>();
    }

    template <typename T, typename Self>
    constexpr auto&& as(this Self&& self)
    {
        auto ptr = self.template as_ptr<T>(); 
        return std::forward_like<Self>(*ptr);
    }

    template <typename Self>
    constexpr auto&& data(this Self&& self)
    {
        return ((Self&&)self).m_data;
    }
};

template <size_t N>
struct to_unique_ptr_if_large_than
{
    template <typename U>
    using type = std::conditional_t<(sizeof(U) > N), std::unique_ptr<U>, U>;

    template <typename T>
    static constexpr auto from_value(T t) 
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
        if constexpr (!meta::is_specialization_of_v<std::remove_cv_t<T>, std::unique_ptr>)
        {
            return t;
        }
        else
        {
            return std::to_address(*t);
        }
    }
};

struct store_self
{
    template <typename U>
    using type = U;

    template <typename T>
    static constexpr auto&& from_value(T&& t)
    {
        return (T&&)t;
    } 

    template <typename T>
    static constexpr auto to_address(T* t) 
    {
        return t;
    }
};


} // namespace leviathan

