#include <iostream>

// #include <lv_cpp/meta/type_list.hpp>
// #include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/meta/function_traits.hpp>

using namespace leviathan::meta;

int func(int, double, char) noexcept { return 0; }

void test_for_function()
{

    static_assert(function_traits<decltype(func)>::is_noexcept == true);

    using Arg1 = function_traits<decltype(func)>::args; 
    using Arg2 = function_traits<decltype(func)>::return_type; 
    using Arg4 = function_traits<decltype(func)>::template nth_arg<0>; 
    // PrintTypeInfo(Arg1);  // (int, int)
    // PrintTypeInfo(Arg2);  // int
    // PrintTypeInfo(Arg4);  // int
    static_assert(std::is_same_v<Arg1, std::tuple<int, double, char>>);
    static_assert(std::is_same_v<Arg2, int>);
    static_assert(std::is_same_v<Arg4, int>);
    const auto& p1 = func;
    auto& p2 = func;
    auto&& p3 = func;
    auto p4 = func;
    using Arg5 = function_traits<decltype(p1)>::args;           
    using Arg6 = function_traits<decltype(p2)>::args;           
    using Arg7 = function_traits<decltype(p3)>::return_type;    
    using Arg8 = function_traits<decltype(p4)>::return_type;    
    
    // PrintTypeInfo(Arg5);  // int
    // PrintTypeInfo(Arg6);  // int
    // PrintTypeInfo(Arg7);  // int
    // PrintTypeInfo(Arg8);  // int
    static_assert(std::is_same_v<Arg5, std::tuple<int, double, char>>);
    static_assert(std::is_same_v<Arg6, std::tuple<int, double, char>>);
    static_assert(std::is_same_v<Arg7, int>);
    static_assert(std::is_same_v<Arg8, int>);
}

void test_for_lambda()
{
    auto a = [](int x){ return x;};
    const auto b = [](double y) { return y; };
    using Arg1 = function_traits<decltype(a)>::args;
    using Arg2 = function_traits<decltype(b)>::args;
    // PrintTypeInfo(Arg1);  // (int)
    // PrintTypeInfo(Arg2);  // (double)
    static_assert(std::is_same_v<Arg1, std::tuple<int>>);
    static_assert(std::is_same_v<Arg2, std::tuple<double>>);
    struct lambda1
    {
        int operator()(int, double) const { return {}; }
    };
    using Arg3 = function_traits<lambda1>::args;
    // PrintTypeInfo(Arg3);  // (int, double)
    static_assert(std::is_same_v<Arg3, std::tuple<int, double>>);
}

template <typename T, typename... Ts>
struct all_type : std::conjunction<std::is_same<T, Ts>...> { };

void test_for_class_member_function()
{
    struct foo
    {
        // 12
        void none(int) noexcept { }
        void const_() const { }
        void const_volatile() const volatile { }
        void volatile_() volatile { }
        
        void ref(int) & { }
        void const_ref(double) const& { }
        void volatile_ref() volatile& noexcept { }
        void const_volatile_ref() const volatile noexcept { }

        void r_ref() && noexcept { }
        void const_r_ref() const&& { }
        void volatile_r_ref() volatile&& { }
        void const_volatile_r_ref() const volatile&& noexcept { }

    };


    using T1 = function_traits<decltype(&foo::none)>::class_type;
    using T2 = function_traits<decltype(&foo::const_)>::class_type;
    using T3 = function_traits<decltype(&foo::volatile_)>::class_type;
    using T4 = function_traits<decltype(&foo::const_volatile)>::class_type;

    using T5 = function_traits<decltype(&foo::ref)>::class_type;
    using T6 = function_traits<decltype(&foo::const_ref)>::class_type;
    using T7 = function_traits<decltype(&foo::volatile_ref)>::class_type;
    using T8 = function_traits<decltype(&foo::const_volatile_ref)>::class_type;

    using T9 = function_traits<decltype(&foo::r_ref)>::class_type;
    using T10 = function_traits<decltype(&foo::const_r_ref)>::class_type;
    using T11 = function_traits<decltype(&foo::volatile_r_ref)>::class_type;
    using T12 = function_traits<decltype(&foo::const_volatile_r_ref)>::class_type;

    // PrintTypeInfo(T1);
    // PrintTypeInfo(T2);
    // PrintTypeInfo(T3);
    // PrintTypeInfo(T4);
    // PrintTypeInfo(T5);
    // PrintTypeInfo(T6);
    // PrintTypeInfo(T7);
    // PrintTypeInfo(T8);
    // PrintTypeInfo(T9);
    // PrintTypeInfo(T10);
    // PrintTypeInfo(T11);
    // PrintTypeInfo(T12);

    static_assert(
        all_type<foo, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>::value
    );

    static_assert(function_traits<decltype(&foo::none)>::is_noexcept == true);
    static_assert(function_traits<decltype(&foo::ref)>::is_noexcept == false);
    static_assert(function_traits<decltype(&foo::ref)>::attribute == 0x0100);
    static_assert(function_traits<decltype(&foo::const_ref)>::attribute == 0x0101);
    static_assert(function_traits<decltype(&foo::const_volatile_r_ref)>::attribute == 0x1011);
    static_assert(function_traits<decltype(&foo::const_)>::attribute == 0x0001);
    static_assert(function_traits<decltype(&foo::none)>::attribute == 0x0000);

}

int main()
{
    test_for_function();
    test_for_lambda();
    test_for_class_member_function();
}