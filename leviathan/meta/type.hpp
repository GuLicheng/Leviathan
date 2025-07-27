#pragma once

#include <string_view>
#include <source_location>

namespace cpp::meta
{

namespace detail
{

template <typename T>
consteval std::string_view type_of()
{
    std::string_view name = std::source_location::current().function_name();

#if defined(_MSC_VER)
    // __FUNCSIG__
    const auto first = name.find("type_of") + sizeof("type_of");
    const auto last = name.rfind("void");
    return name.substr(first, last - first - 2);

#elif defined(__clang__)
    // __PRETTY_FUNCTION__
    // consteval std::string_view type_of() [T = int]
    const auto first = name.find_first_of('=') + 2;
    const auto last = name.find_last_of(']');
    return name.substr(first, last - first);

#elif defined(__GNUC__)
    // __PRETTY_FUNCTION__
    // consteval std::string_view type_of() [with T = std::__cxx11::basic_string<char>; std::string_view = std::basic_string_view<char>]
    const auto first = name.find_first_of('=') + 2;
    const auto last = name.find_last_of(';');
    return name.substr(first, last - first);

#else
    return name;
#endif
}

} // namespace detail

template <typename T>
inline constexpr std::string_view name_of = detail::type_of<T>();

} // namespace cpp::meta

