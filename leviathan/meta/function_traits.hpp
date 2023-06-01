#ifndef __FUNCTION_TRAITS_HPP__
#define __FUNCTION_TRAITS_HPP__

#include <tuple>

// for function_traits
namespace leviathan::meta
{

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

            constexpr static bool is_noexcept = IsNoThrow;

            constexpr static unsigned attribute = Attribute;
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
        if you want support variadic paramsters such as `int printf(const char*, ...)` please add it's specialization
    */

    // int(*)(int, int)
    template <typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(*)(Args...) noexcept(IsNoThrow)> 
        : detail::function_traits_impl<0, IsNoThrow, void, R, Args...> { };

    // int(&)(int, int)
    template <typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(&)(Args...) noexcept(IsNoThrow)> 
        : detail::function_traits_impl<0, IsNoThrow, void, R, Args...> { };

    // int(int, int)
    template <typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(Args... ) noexcept(IsNoThrow)> 
        : detail::function_traits_impl<0, IsNoThrow, void, R, Args...> { };

    // int printf(const char*, ...)
    template <typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(Args......) noexcept(IsNoThrow)> 
        : detail::function_traits_impl<0, IsNoThrow, void, R, Args...> { };

    // no any cv_ref
    template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> 
        : detail::function_traits_impl<0, IsNoThrow, ClassType, R, Args...> { };

    // const
    template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const noexcept(IsNoThrow)> 
        : detail::function_traits_impl<0x0001, IsNoThrow, ClassType, R, Args...> { };

    // volatile
    template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) volatile noexcept(IsNoThrow)> 
        : detail::function_traits_impl<0x0010, IsNoThrow, ClassType, R, Args...> { };


    // const volatile
    template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const volatile noexcept(IsNoThrow)> 
        : detail::function_traits_impl<0x0011, IsNoThrow, ClassType, R, Args...> { };

    // const&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const& noexcept(IsNoThrow)>
        : detail::function_traits_impl<0x0101, IsNoThrow, ClassType, R, Args...> { };

    // &
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) & noexcept(IsNoThrow)>
        : detail::function_traits_impl<0x0100, IsNoThrow, ClassType, R, Args...> { };

    // volatile&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) volatile& noexcept(IsNoThrow)>
        : detail::function_traits_impl<0x0110, IsNoThrow, ClassType, R, Args...> { };

    // const volatile&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const volatile& noexcept(IsNoThrow)>
        : detail::function_traits_impl<0x0111, IsNoThrow, ClassType, R, Args...> { };

    // &&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) && noexcept(IsNoThrow)>
        : detail::function_traits_impl<0x1000, IsNoThrow, ClassType, R, Args...> { };

    // const&&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const&& noexcept(IsNoThrow)>
        : detail::function_traits_impl<0x1001, IsNoThrow, ClassType, R, Args...> { };

    // volatile&&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) volatile&& noexcept(IsNoThrow)>
        : detail::function_traits_impl<0x1010, IsNoThrow, ClassType, R, Args...> { };

    // const volatile&&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const volatile&& noexcept(IsNoThrow)>
        : detail::function_traits_impl<0x1011, IsNoThrow, ClassType, R, Args...> { };


    // specialize for operator()
    template <typename T>
    struct function_traits : function_traits<decltype(&T::operator())> { };


} // leviathan::meta








#endif
