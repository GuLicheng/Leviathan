#include <iostream>

#include <lv_cpp/type_list.hpp>
#include <lv_cpp/template_info.hpp>

using namespace leviathan::meta;

int func(int, double, char) { return 0; }

void test_for_function()
{
    using Arg1 = function_traits<decltype(func)>::args; 
    using Arg2 = function_traits<decltype(func)>::return_type; 
    using Arg4 = function_traits<decltype(func)>::template nth_arg<0>; 
    PrintTypeInfo(Arg1);  // (int, int)
    PrintTypeInfo(Arg2);  // int
    PrintTypeInfo(Arg4);  // int

    const auto& p1 = func;
    auto& p2 = func;
    auto&& p3 = func;
    auto p4 = func;
    using Arg5 = function_traits<decltype(p1)>::args;           
    using Arg6 = function_traits<decltype(p2)>::args;           
    using Arg7 = function_traits<decltype(p3)>::return_type;    
    using Arg8 = function_traits<decltype(p4)>::return_type;    
    
    PrintTypeInfo(Arg5);  // int
    PrintTypeInfo(Arg6);  // int
    PrintTypeInfo(Arg7);  // int
    PrintTypeInfo(Arg8);  // int
}

void test_for_lambda()
{
    auto a = [](int x){ return x;};
    const auto b = [](double y) { return y; };
    using Arg1 = function_traits<decltype(a)>::args;
    using Arg2 = function_traits<decltype(b)>::args;
    PrintTypeInfo(Arg1);  // (int)
    PrintTypeInfo(Arg2);  // (double)

    struct lambda1
    {
        int operator()(int, double) const { return {}; }
    };
    using Arg3 = function_traits<lambda1>::args;
    PrintTypeInfo(Arg3);  // (int, double)
}

int main()
{
    test_for_function();
    test_for_lambda();
}