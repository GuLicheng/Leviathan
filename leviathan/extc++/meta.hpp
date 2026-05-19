#pragma once

#include <meta>
#include <format>
#include <type_traits>
#include <string_view>
#include <stdexcept>
#include <string>
#include <vector>
#include <leviathan/annotations/all.hpp>

namespace cpp::refl
{

/**
 * @brief Get the N-th member of a class by its declaration order.
 * @tparam T The class type.
 * @param N The index of the member, starting from 0.
 * @return The meta-information of the N-th member.
 * 
 * For example, given a class:
 * 
 *  struct MyStruct { int X; double Y; };
 *  MyStruct s;
 *  s.[:member_number<MyStruct>(0):] = 1;
 *  s.[:member_number<MyStruct>(1):] = 3.14;
 *  assert(s.X == 1 && s.Y == 3.14);
 */
template <typename T>
consteval std::meta::info member_number(size_t N)
{
    auto ctx = std::meta::access_context::current();
    return std::meta::nonstatic_data_members_of(^^T, ctx)[N];
}

/**
 * @brief Get the member of a class by its name.
 * @tparam T The class type.
 * @param name The name of the member.
 * @return The meta-information of the member with the given name.
 * 
 *  For example, given a class:
 *  struct MyStruct { int X; double Y; };
 *  MyStruct s;
 *  s.[:member_named<MyStruct>("X"):] = 1;
 *  s.[:member_named<MyStruct>("Y"):] = 3.14;
 *  assert(s.X == 1 && s.Y == 3.14);
 */
template <typename T>
consteval std::meta::info member_named(const char* name)
{
    auto ctx = std::meta::access_context::current();
    for (std::meta::info field : nonstatic_data_members_of(^^T, ctx))
        if (has_identifier(field) && identifier_of(field) == name)
            return field;
    throw std::runtime_error(std::format("No member named {} in type {}", name, identifier_of(^^T)));
}

template <std::meta::info ClassInfo, std::meta::info FieldInfo>
struct field_handler
{
    static_assert(std::meta::is_class_type(ClassInfo) && std::meta::is_class_member(FieldInfo));

    using FieldType = typename [:type_of(FieldInfo):];

    static constexpr bool IsDefaultConstructible = std::is_default_constructible_v<FieldType>;

    static constexpr bool IsSkippable = cpp::refl::has_annotation(FieldInfo, cpp::refl::skip, cpp::refl::skip_deserialization);

    template <typename Initializer>
    static constexpr std::optional<FieldType> init_value(Initializer initializer)
    {
        if constexpr (IsSkippable)
        {
            return default_value();
        }
        else
        {
            // Get field name
            auto name = extract_name_by_annotation<FieldInfo, ClassInfo>();
            std::optional<FieldType> value = std::nullopt;

            // Try init current field with initializer
            std::invoke(initializer, value, name);

            // Try init current field with annotations
            return value ? value : default_value();
        }
    }

    static constexpr std::optional<FieldType> default_value() 
    {
        template for (constexpr auto anno : define_static_array(annotations_of(FieldInfo)))
            if constexpr (has_annotation(type_of(anno), value_annotation))
                return std::make_optional(std::invoke(extract<typename [:type_of(anno):]>(anno)));
        if constexpr (IsDefaultConstructible)
            return std::make_optional(FieldType());
        else
            return std::nullopt;
    }

    template <typename T>
    static constexpr bool is_valid(const T& value)
    {
        template for (constexpr auto anno : define_static_array(annotations_of(FieldInfo)))
            if constexpr (has_annotation(type_of(anno), choice_annotation))
                if (!std::invoke(extract<typename [:type_of(anno):]>(anno), value))
                    return false;
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

/**
 * @brief Construct an object of type T by initializing its fields with the provided initializer.
 * @param T The type of the object to construct. Must be a class type.
 * @param Initializer A callable that takes a reference to an optional field value 
 *  and the field name, and initializes the field value if possible.
 */
template <typename T, typename Initializer>
constexpr T construct_struct(Initializer initializer)
{
    constexpr auto ctx = std::meta::access_context::current();
    constexpr auto members = define_static_array(nonstatic_data_members_of(^^T, ctx));
    constexpr auto N = members.size();
    constexpr auto [...Idx] = std::make_index_sequence<N>{};
    return T(cpp::refl::field_handler<^^T, members[Idx]>()(std::ref(initializer))...);
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

