#ifndef __FUNCTION_TRAITS_HPP__
#define __FUNCTION_TRAITS_HPP__

// for function_traits
namespace leviathan::meta
{

    namespace detail
    {
        
        template <bool IsNoThrow, typename R, typename... Args>
        struct function_traits_impl
        {
            static constexpr auto argc = sizeof...(Args);

            using return_type = R;

            using args = std::tuple<Args...>;

            template <std::size_t N>
            using nth_arg = std::tuple_element_t<N, std::tuple<Args...>>;

            constexpr static bool is_noexcept = IsNoThrow;

        };

    } // namespace detail

    template <typename T>
    struct function_traits;
    /*
        function x 3
        member_function x 12
        not support overload function
    */

    // int(*)(int, int)
    template <typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(*)(Args...) noexcept(IsNoThrow)> 
        : detail::function_traits_impl<IsNoThrow, R, Args...> { };

    // int(&)(int, int)
    template <typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(&)(Args...) noexcept(IsNoThrow)> 
        : detail::function_traits_impl<IsNoThrow, R, Args...> { };

    // int(int, int)
    template <typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(Args... ) noexcept(IsNoThrow)> 
        : detail::function_traits_impl<IsNoThrow, R, Args...> { };


    // no any cv_ref
    template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> 
        : detail::function_traits_impl<IsNoThrow, R, Args...>
    {
        using class_type = ClassType;
    };

    // const
    template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const noexcept(IsNoThrow)> 
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

    // volatile
    template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) volatile noexcept(IsNoThrow)> 
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };


    // const volatile
    template <typename ClassType, typename R, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const volatile noexcept(IsNoThrow)> 
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

    // const&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const& noexcept(IsNoThrow)>
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

    // &
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) & noexcept(IsNoThrow)>
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

    // volatile&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) volatile& noexcept(IsNoThrow)>
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

    // const volatile&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const volatile& noexcept(IsNoThrow)>
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

    // &&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) && noexcept(IsNoThrow)>
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

    // const&&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const&& noexcept(IsNoThrow)>
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

    // volatile&&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) volatile&& noexcept(IsNoThrow)>
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };

    // const volatile&&
    template <typename R, typename ClassType, bool IsNoThrow, typename... Args>
    struct function_traits<R(ClassType::*)(Args...) const volatile&& noexcept(IsNoThrow)>
        : function_traits<R(ClassType::*)(Args...) noexcept(IsNoThrow)> { };



    // specialize for operator()
    template <typename T>
    struct function_traits : function_traits<decltype(&T::operator())> {};


} // leviathan::meta








#endif
