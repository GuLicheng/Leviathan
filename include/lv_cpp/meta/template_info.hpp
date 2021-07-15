/*
    this header provide some macro for getting the type of instance
*/

#ifndef __TEMPLATE_INFO_HPP__
#define __TEMPLATE_INFO_HPP__

#include <string_view>

// Only Support cl, gcc and clang
template <typename T>
constexpr std::string_view type_to_str() noexcept
{
#if defined(_MSC_VER)
    // class std::basic_string_view<char,struct std::char_traits<char> > __cdecl 
    // type_to_str<int&>(void)
    std::string_view info(__FUNCSIG__);
    const auto first = info.find("type_to_str") + sizeof("type_to_str");
    const auto last = info.rfind("void");
    return info.substr(first, last - first - 2);

#elif defined(__clang__)
    // std::string_view get_type() [T = int]
    std::string_view info{__PRETTY_FUNCTION__};
    const auto first = info.find_first_of('=') + 2;
    const auto last = info.find_last_of(']');
    return info.substr(first, last - first);

#elif defined(__GNUC__)
    // constexpr std::string_view get_type() 
    // [with T = int; std::string_view = std::basic_string_view<char>]
    std::string_view info{__PRETTY_FUNCTION__};
    const auto first = info.find_first_of('=') + 2;
    const auto last = info.find_last_of(';');
    return info.substr(first, last - first);

#else
#error "not support"
#endif
}

#define STR(info) #info

#define GetTypeCategory(type_instance) type_to_str<decltype(type_instance)>()
#define GetValueCategory(type_instance) type_to_str<decltype((type_instance))>()


#define TEMPLATE_INFORMATION() (std::cout << __PRETTY_FUNCTION__ << std::endl)

#define PrintTypeCategory(type_instance) \
    (std::cout << "The type category of "<< STR(type_instance) << " is: " \
        << GetTypeCategory(type_instance) << std::endl)

#define PrintValueCategory(type_instance) \
    (std::cout << "The value category of " << STR(type_instance) << " is: " \
        << GetValueCategory(type_instance) << std::endl)

#define DivingLine() \
    (std::cout << "==========================================" << std::endl)

#define PrintInstanceWithLine(instance) \
    (std::cout << "===========" << instance << "=============" << std::endl)

/*
    1. for multi-parameters: using T = std::tuple<...> PrintTypeInfo(T)
    2. template <typename... Ts> void func(Ts... ts) { (PrintTypeInfo(Ts), ...); }
*/
#define PrintTypeInfo(type) \
    (std::cout << type_to_str< type >() << std::endl)



#endif