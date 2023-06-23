#pragma once

#include <optional>
#include <type_traits>

#include <assert.h>

namespace leviathan::config
{
    using std::nullopt_t;
    using std::nullopt;
    using std::bad_optional_access;

    template <typename T> class optional;

    template <typename T>
    struct is_optional : std::false_type { };

    template <typename T>
    struct is_optional<optional<T>> : std::true_type { };

    template <typename T>
    inline constexpr bool is_optional_v = is_optional<T>::value;

    // Extend optional for T&
    template <typename T>
    class optional : public std::optional<T> 
    {
        using std::optional<T>::optional;
        using std::optional<T>::operator=;
    };

    // https://www.boost.org/doc/libs/1_79_0/libs/optional/doc/html/boost_optional/reference/header__boost_optional_optional_hpp_/header_optional_optional_refs.html#reference_operator_template_spec
    // std::optional<T&> specialization for lvalue references
    template <typename T>
    class optional<T&>
    {
        static_assert(std::is_lvalue_reference_v<T>);

        typedef T& value_type;
        typedef T& reference_type;
        typedef T& reference_const_type; // no const propagation
        typedef T& rval_reference_type;
        typedef T* pointer_type;
        typedef T* pointer_const_type;   // no const propagation

        // Postconditions: bool(*this) == false; *this refers to nothing.
        constexpr optional() = default;

        constexpr optional(nullopt_t) : optional() { }

        template <typename R> 
            requires (!is_optional_v<std::decay_t<R>>)
        constexpr optional(R&& r) 
        {
            assert(std::is_lvalue_reference_v<R> && "Unless R is an lvalue reference, the program is ill-formed");
        }

        template <typename R> 
            requires (!is_optional_v<std::decay_t<R>>)
        constexpr optional(bool cond, R&& r)
            : m_ref(cond ? std::addressof(r) : nullptr)
        {
            assert(std::is_lvalue_reference_v<R> && "Unless R is an lvalue reference, the program is ill-formed");
        }

        constexpr optional(const optional& rhs) = default;

        template <typename U>
            requires (std::is_convertible_v<U&, T&>) 
        constexpr explicit optional(const optional<U&>& rhs )
            : m_ref(rhs.m_ref)
        {
        }

        constexpr optional& operator=(nullopt_t)
        {
            m_ref = nullptr;
            return *this;
        }

        constexpr optional& operator=(const optional& rhs)
        {
            m_ref = rhs.m_ref;
            return *this;
        }

        template <typename U> 
            requires (std::is_convertible_v<U&, T&>) 
        constexpr optional& operator=( const optional<U&>& rhs)
        {
            m_ref = rhs.m_ref;
            return *this;
        }

        template <typename R> 
            requires (!is_optional_v<std::decay_t<R>>)
        constexpr optional& operator=(R&& r)
        {
            assert(std::is_lvalue_reference_v<R> && "Unless R is an lvalue reference, the program is ill-formed");
            m_ref = std::addressof(r);
            return *this;
        }

        template <typename R> 
            requires (!is_optional_v<std::decay_t<R>>)
        constexpr void emplace(R&& r)
        {
            assert(std::is_lvalue_reference_v<R> && "Unless R is an lvalue reference, the program is ill-formed");
            m_ref = std::addressof(r);
        }

        constexpr T& get() const
        { return **this; }

        constexpr T& operator*() const
        {            
            assert(m_ref && "ref should not be nullptr");
            return *m_ref;
        }

        constexpr T* operator ->() const
        { return m_ref; }

        constexpr T& value() const&
        { return m_ref ? *m_ref : throw bad_optional_access(); }

        template <typename R> 
        constexpr T& value_or(R&& r ) const
        {
            if (*this)
                return **this;
            return r;
        }

        template <typename F> 
        constexpr T& value_or_eval(F f) const
        {
            if (*this)
                return **this;
            return f();
        }

        template <typename F> 
        constexpr auto map(F f) const -> optional<decltype(f(**this))>
        {
            if (*this)
                return f(**this);
            return nullopt;
        }

        // template <typename F> auto flat_map(F f) const;

        constexpr T* get_ptr() const 
        { return m_ref; }

        constexpr bool has_value() const
        { return bool(m_ref); }

        constexpr explicit operator bool() const 
        { return bool(m_ref); }

        constexpr bool operator!() const
        { return !bool(m_ref); }

        constexpr void reset() 
        { m_ref = nullptr; }

    private:

        T* m_ref = nullptr; // exposition only
    };


}
