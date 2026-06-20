#pragma once

#include <tuple>

// for function_traits
namespace cpp::meta
{

enum function_type : unsigned
{
    c_function             = 0b00000000,
    c_variadic_function    = 0b10000000,
    
    cxx_class              = 0b00000001,
    cxx_class_const        = 0b00000011,
    cxx_class_volatile     = 0b00000101,
    cxx_class_lvalue_ref   = 0b00001001,
    cxx_class_rvalue_ref   = 0b00010001,
    cxx_class_member_field = 0b00100001,
};

using enum function_type;

namespace detail
{
    
template <unsigned Attribute, bool IsNoThrow, typename ClassType, typename R, typename... Args>
struct function_traits_impl
{
    static constexpr auto argc = sizeof...(Args);

    using return_type = R;

    using args = std::tuple<Args...>;

    using class_type = ClassType; // for non-class type, it will be void

    template <std::size_t N>
    using nth_arg = std::tuple_element_t<N, std::tuple<Args...>>;

    static constexpr bool is_noexcept = IsNoThrow;

    static constexpr function_type attribute = Attribute;
    /*
        0000 for none
        0001 for const
        0010 for volatile
        0100 for &
        1000 for &&
    */
};

} // namespace detail

template <typename T>
struct function_traits;
/*
    function x 3 
    member_function x 12
    total: (3 + 12)  x2(if necessary) 
    not support overload function
    if you want support variadic parameters such as `int printf(const char*, ...)` please add it's specialization
*/

// int(*)(int, int)
template <typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(*)(Args...) noexcept(IsNoThrow)> 
    : detail::function_traits_impl<c_function, IsNoThrow, void, R, Args...> { };

// int(&)(int, int)
template <typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(&)(Args...) noexcept(IsNoThrow)> 
    : detail::function_traits_impl<c_function, IsNoThrow, void, R, Args...> { };

// int(int, int)
template <typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(Args... ) noexcept(IsNoThrow)> 
    : detail::function_traits_impl<c_function, IsNoThrow, void, R, Args...> { };

// int printf(const char*, ...)
template <typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(Args......) noexcept(IsNoThrow)> 
    : detail::function_traits_impl<c_variadic_function, IsNoThrow, void, R, Args...> { };

// no any cv_ref
template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> 
    : detail::function_traits_impl<cxx_class, IsNoThrow, ClassType, R, Args...> { };

// const
template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const noexcept(IsNoThrow)> 
    : detail::function_traits_impl<cxx_class_const, IsNoThrow, ClassType, R, Args...> { };

// volatile
template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) volatile noexcept(IsNoThrow)> 
    : detail::function_traits_impl<cxx_class_volatile, IsNoThrow, ClassType, R, Args...> { };

// const volatile
template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const volatile noexcept(IsNoThrow)> 
    : detail::function_traits_impl<cxx_class_const | cxx_class_volatile, IsNoThrow, ClassType, R, Args...> { };

// const&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const& noexcept(IsNoThrow)>
    : detail::function_traits_impl<cxx_class_const | cxx_class_lvalue_ref, IsNoThrow, ClassType, R, Args...> { };

// &
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) & noexcept(IsNoThrow)>
    : detail::function_traits_impl<cxx_class_lvalue_ref, IsNoThrow, ClassType, R, Args...> { };

// volatile&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) volatile& noexcept(IsNoThrow)>
    : detail::function_traits_impl<cxx_class_volatile | cxx_class_lvalue_ref, IsNoThrow, ClassType, R, Args...> { };

// const volatile&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const volatile& noexcept(IsNoThrow)>
    : detail::function_traits_impl<cxx_class_const | cxx_class_volatile | cxx_class_lvalue_ref, IsNoThrow, ClassType, R, Args...> { };

// &&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) && noexcept(IsNoThrow)>
    : detail::function_traits_impl<cxx_class_rvalue_ref, IsNoThrow, ClassType, R, Args...> { };

// const&&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const&& noexcept(IsNoThrow)>
    : detail::function_traits_impl<cxx_class_const | cxx_class_rvalue_ref, IsNoThrow, ClassType, R, Args...> { };

// volatile&&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) volatile&& noexcept(IsNoThrow)>
    : detail::function_traits_impl<cxx_class_volatile | cxx_class_rvalue_ref, IsNoThrow, ClassType, R, Args...> { };

// const volatile&&
template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
struct function_traits<R(ClassType::*)(Args...) const volatile&& noexcept(IsNoThrow)>
    : detail::function_traits_impl<cxx_class_const | cxx_class_volatile | cxx_class_rvalue_ref, IsNoThrow, ClassType, R, Args...> { };

// cpp member field, the argc is 1 and type can be one of [C, const C, ...]
template <typename ClassType, typename R>
struct function_traits<R ClassType::*> 
    : detail::function_traits_impl<cxx_class_member_field, true, ClassType, R, void> { };

// specialize for operator()
template <typename T>
struct function_traits : function_traits<decltype(&T::operator())> { };


} // cpp::meta








