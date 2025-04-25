#pragma once

#include <string_view>
#include <source_location>

namespace cpp::meta
{
    
template <typename T>
consteval std::string_view type_name()
{
    std::string_view name = std::source_location::current().function_name();

#if defined(_MSC_VER)
    // __FUNCSIG__
    const auto first = name.find("type_name") + sizeof("type_name");
    const auto last = name.rfind("void");
    return name.substr(first, last - first - 2);

#elif defined(__clang__)
    // __PRETTY_FUNCTION__
    // consteval std::string_view type_name() [T = int]
    const auto first = name.find_first_of('=') + 2;
    const auto last = name.find_last_of(']');
    return name.substr(first, last - first);

#elif defined(__GNUC__)
    // __PRETTY_FUNCTION__
    // consteval std::string_view type_name() [with T = std::__cxx11::basic_string<char>; std::string_view = std::basic_string_view<char>]
    const auto first = name.find_first_of('=') + 2;
    const auto last = name.find_last_of(';');
    return name.substr(first, last - first);

#else
    return name;
#endif
}

} // namespace cpp::meta

