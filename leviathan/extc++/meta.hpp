#pragma once

#include <meta>
#include <type_traits>
#include <string_view>
#include <stdexcept>
#include <string>
#include <vector>
#include <leviathan/annotations/all.hpp>

namespace cpp::refl
{

template <typename T>
consteval auto member_number(size_t N)
{
    auto ctx = std::meta::access_context::current();
    return std::meta::nonstatic_data_members_of(^^T, ctx)[N];
}

template <typename T>
consteval auto member_named(const char* name)
{
    auto ctx = std::meta::access_context::current();
    for (std::meta::info field : nonstatic_data_members_of(^^T, ctx))
        if (has_identifier(field) && identifier_of(field) == name)
            return field;
    throw std::runtime_error(std::format("No member named {} in type {}", name, identifier_of(^^T)));
}

template <std::meta::info ClassInfo, std::meta::info FieldInfo>
struct field_initializer
{
    static_assert(std::meta::is_class_type(ClassInfo) && std::meta::is_class_member(FieldInfo));

    using FieldType = typename [:type_of(FieldInfo):];

    static constexpr bool IsDefaultConstructible = std::is_default_constructible_v<FieldType>;

    template <typename Initializer>
    static constexpr std::optional<FieldType> init_value(Initializer initializer)
    {
        // Get field name
        auto name = extract_name_by_annotation<FieldInfo, ClassInfo>();
        std::optional<FieldType> value = std::nullopt;

        // Try init current field with initializer
        std::invoke(initializer, value, name);

        // Try init current field with annotations
        if (!value)
        {
            template for (constexpr auto anno : define_static_array(annotations_of(FieldInfo)))
            {
                if constexpr (has_annotation(type_of(anno), value_annotation))
                {
                    value.emplace(std::invoke(extract<typename [:type_of(anno):]>(anno)));
                    break;  // Only the first value annotation is effective
                }
            }  

            if (!value)
            {
                if constexpr (!IsDefaultConstructible)
                {
                    throw std::runtime_error(std::format("Field {} is missing and has no default value", name));
                }
                else
                {
                    value.emplace(); // default initialize
                }
            }
        }
        return value;
    }

    template <typename T>
    static constexpr bool is_valid(const T& value)
    {
        template for (constexpr auto anno : define_static_array(annotations_of(FieldInfo)))
        {
            using AnnoType = typename [:type_of(anno):];

            if constexpr (has_annotation(type_of(anno), choice_annotation))
            {
                if (!std::invoke(extract<AnnoType>(anno), value))
                {
                    return false;
                }
            }
        }

        return true;
    }

    template <typename Initializer>
    static constexpr FieldType operator()(Initializer initializer)
    {
        auto value = init_value(initializer);
        return value.has_value() && is_valid(*value) 
             ? std::move(*value) 
             : throw std::runtime_error(std::format("Field {} is missing or invalid", display_string_of(FieldInfo)));
    }
};

template <typename T, typename Initializer>
constexpr T construct_struct(Initializer initializer)
{
    constexpr auto ctx = std::meta::access_context::current();
    constexpr auto members = define_static_array(nonstatic_data_members_of(^^T, ctx));
    constexpr auto N = members.size();
    constexpr auto [...Idx] = std::make_index_sequence<N>{};
    return T(cpp::refl::field_initializer<^^T, members[Idx]>()(std::ref(initializer))...);
}

template <typename T>
constexpr auto struct_to_tuple(const T& t) 
{
    constexpr auto ctx = std::meta::access_context::current();
    constexpr auto members = define_static_array(nonstatic_data_members_of(^^T, ctx));
    constexpr auto N = members.size();
    constexpr auto [...Is] = std::make_index_sequence<N>{};
    return std::make_tuple(t.[:members[Is]:]...);
}


} // namespace cpp::refl

