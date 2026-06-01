/*
    - lowercase
    - UPPERCASE
    - PascalCase
    - camelCase
    - snake_case (default)
    - SCREAMING_SNAKE_CASE
    - kebab-case
    - SCREAMING-KEBAB-CASE
*/
#pragma once

#include <leviathan/annotations/common.hpp>
#include <ranges>
#include <meta>

namespace cpp::refl
{

template <typename F>
struct [[=rename_annotation]] function_rename_annotation : callable<F>
{
    using callable<F>::callable;
    using callable<F>::operator();
};

inline constexpr auto shortname = function_rename_annotation([](std::string field_name) static 
{
    // assert(!name.empty(), "Name cannot be empty");
    return '-' + std::string(field_name.begin(), field_name.begin() + 1);
});

inline constexpr auto longname = function_rename_annotation([](std::string field_name) static 
{
    // assert(!name.empty(), "Name cannot be empty");
    return "--" + std::string(field_name);
});

inline constexpr auto lowercase = function_rename_annotation([](std::string field_name) static 
{
    return field_name | std::views::transform(::tolower) | std::ranges::to<std::string>();
});

inline constexpr auto uppercase = function_rename_annotation([](std::string field_name) static 
{
    return field_name | std::views::transform(::toupper) | std::ranges::to<std::string>();
});

inline constexpr auto rename = [](std::string_view new_name) static
{
    return function_rename_annotation([name=define_static_string(new_name)](auto&&...)  
    {
        return std::string(name);
    });
};

// Follows functions in terms of implementation maybe incorrect
// FIXME: Rust clap-
inline constexpr auto camel_case = function_rename_annotation([](std::string field_name) static
{
    std::string out;
    bool upper_next = false;
    for (char c : field_name) {
        if (c == '_') { upper_next = true; continue; }
        if (upper_next && c >= 'a' && c <= 'z')
            out += static_cast<char>(c - ('a' - 'A'));
        else
            out += c;
        upper_next = false;
    }
    return out;
});

inline constexpr auto pascal_case = function_rename_annotation([](std::string field_name) static
{
    auto upper_first_character = [](auto&& part) static {
        if (!part.empty()) part.front() = ::toupper(part.front());
        return part;
    };

    return field_name 
         | std::views::split('_') 
         | std::views::transform(upper_first_character)
         | std::views::join
         | std::ranges::to<std::string>();
});

inline constexpr auto kebab_case = function_rename_annotation([](std::string field_name) static
{
    return field_name | std::views::transform([](char c) { return c == '_' ? '-' : c; }) | std::ranges::to<std::string>();
});

namespace detail
{

template <std::meta::info Info1, std::meta::info... Infos>
struct extract_name_by_annotation_impl
{
    static constexpr std::string operator()(std::string name) requires (sizeof...(Infos) == 0)
    {
        template for (constexpr auto anno : define_static_array(annotations_of(Info1)))
            if constexpr (has_annotation(type_of(anno), rename_annotation))
                name = std::invoke(extract<typename [:type_of(anno):]>(anno), name);
        return name;   
    }

    static constexpr std::string operator()(std::string name)
    {
        bool found = false;

        template for (constexpr auto anno : define_static_array(annotations_of(Info1)))
        {
            if constexpr (has_annotation(type_of(anno), rename_annotation))
            {
                name = std::invoke(extract<typename [:type_of(anno):]>(anno), name);
                found = true;
            }
        }
        return found ? name : extract_name_by_annotation_impl<Infos...>()(name);
    }
};

}  // namespace detail

template <std::meta::info Info1, std::meta::info... Infos>
constexpr std::string extract_name_by_annotation()
{
    constexpr auto name = identifier_of(Info1);
    return detail::extract_name_by_annotation_impl<Info1, Infos...>::operator()(std::string(name));
}


}  // namespace cpp::refl
