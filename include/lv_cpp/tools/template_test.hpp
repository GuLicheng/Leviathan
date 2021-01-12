#ifndef __TEMPLATE_TEST_HPP__
#define __TEMPLATE_TEST_HPP__

#include "template_info.hpp"

#include <tuple>


template <template <typename...> typename Fun, typename... Paras>
struct Call_
{
    using T = std::tuple<typename Fun<Paras>::type ...>;
};

template <template <typename...> typename Fun, typename... Paras>
struct Call_<Fun, std::tuple<Paras...>>
{
    using type = std::tuple<typename Fun<Paras>::type ...>;
};

template <template <typename L> typename Fun, typename List>
struct Call_List;

template <template <typename L> typename Fun, typename... Ts>
struct Call_List<Fun, std::tuple<Ts...>>
{
    using type = typename Fun<std::tuple<Ts...>>::type;
};

template <template <typename L> typename Fun, typename... Paras>
struct Call_Func
{
    using type = typename Fun<Paras...>::type;
};

using __T1 = std::tuple<
                    int, const int, volatile int, const volatile int, 
                    int&, int&&, const int&, const int&&,
                    int*, const int*, int[], int[1]
                    >;

using __T2 = std::pair<int, std::pair<int, std::pair<int, std::pair<int, int>>>>;
using __T3 = std::tuple<int8_t, int16_t, int32_t, int64_t, float, double, long double>;

// the template_called must have member type aligns "type" for result
#define GetRes1(template_called) Call_<template_called, __T1>::type
#define GetRes2(template_called) Call_<template_called, __T2>::type
#define GetRes3(template_called) Call_<template_called, __T3>::type

#define GetResList1(template_called) Call_List<template_called, __T1>::type
#define GetResList2(template_called) Call_List<template_called, __T2>::type
#define GetResList3(template_called) Call_List<template_called, __T3>::type

#endif