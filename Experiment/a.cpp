#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <lv_cpp/math/algorithm.hpp>
#include <lv_cpp/meta/function_traits.hpp>

template <typename F>
struct T : std::type_identity<bool> { };

template <typename Res, typename... Args>
struct T<Res(*)(Args......)> : std::type_identity<Res> { };

void f(int) { }

int main()
{
    using T = decltype(::printf);
    typename leviathan::meta::function_traits<T>::nth_arg<0> i = "Hello";
    ::printf(i);
}

