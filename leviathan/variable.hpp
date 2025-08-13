#pragma once

#include <leviathan/extc++/concepts.hpp>
#include <leviathan/type_caster.hpp>
#include <functional>
#include <type_traits>
#include <variant>
#include <memory>

namespace cpp
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

    // The std::variant will hold the first type for default ctor.
    constexpr variable() = default;

    template <typename Arg>
        requires (declaration<Arg>::value)
    constexpr variable(Arg arg) : m_data(Fn::from_value(std::move(arg))) { }

    template <typename Arg> 
        requires (declaration<Arg>::value)
    constexpr variable& operator=(Arg arg)
    {
        m_data = Fn::from_value(std::move(arg));
        return *this;
    }

    constexpr variable(const variable&) = delete("not support default copy constructor");
    constexpr variable& operator=(const variable&) = delete("not support default copy assignment operator");
    
    constexpr variable(variable&&) = default;
    constexpr variable& operator=(variable&&) = default;

    // Compare each type directly may be incorrect. For example, if lhs 
    // contains 'int' and rhs contains 'double', the lhs('int') can directly
    // compared with rhs('double'), but they hold different types.   
    // constexpr bool operator==(const variable& rhs) const;

    template <typename T, typename... Args>
    void emplace(Args&&... args)
    {
        constexpr auto idx = meta::index<T, Ts...>();
        m_data.template emplace<idx>(Fn::from_value(T((Args&&) args...)));
    }

    constexpr size_t index() const 
    {
        return m_data.index();
    }

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

} // namespace cpp

