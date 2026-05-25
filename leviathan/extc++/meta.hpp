#pragma once

#include <meta>
#include <format>
#include <type_traits>
#include <string_view>
#include <stdexcept>
#include <string>
#include <vector>
#include <leviathan/annotations/all.hpp>
#include <leviathan//type_caster.hpp>

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
struct field_handler;

/**
 * @brief Get the valid member indices of a class, excluding the members with [[=cpp::refl::skip]] annotation.
 * @tparam T The class type.
 * 
 * For example, given a class:
 *  struct MyStruct { int X; [[=cpp::refl::skip]] double Y; char Z; };
 *  The valid member indices of MyStruct are 0 and 2, while index 1 is skipped due to the annotation.
 */
template <typename T, auto... Annotations>
consteval std::meta::info remove_skiped_member()
{
    std::vector args { ^^size_t };
    constexpr auto ctx = std::meta::access_context::current();
    constexpr auto member = define_static_array(nonstatic_data_members_of(^^T, ctx));
    constexpr auto size = member.size();
    constexpr auto [...indices] = std::make_index_sequence<size>{};

    auto pusher = [=, &args]<size_t idx>() {
        if constexpr (!cpp::refl::has_annotation(member[idx], Annotations...))
            args.push_back(std::meta::reflect_constant(idx));
    };

    (pusher.template operator()<indices>(), ...);
    return substitute(^^std::integer_sequence, args);
}

template <typename T, auto... Annotations>
using indices_without_removed_member = typename [:remove_skiped_member<T, Annotations...>():];

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

    // base class
    constexpr auto bases = define_static_array(bases_of(^^T, ctx));
    constexpr auto M = bases.size();
    constexpr auto [...base_indices] = std::make_index_sequence<M>();

    // current
    constexpr auto members = define_static_array(nonstatic_data_members_of(^^T, ctx));
    constexpr auto N = members.size();
    constexpr auto [...indices] = std::make_index_sequence<N>();

    // Init base class first and then init current class, since base class is usually
    // used as part of current class's field initialization.
    return T(
        cpp::refl::construct_struct<typename [:type_of(bases[base_indices]):]>(std::ref(initializer))...,
        cpp::refl::field_handler<^^T, members[indices]>()(std::ref(initializer))...
    );
}

/**
 * @brief Convert a struct to a tuple by its members.
 * @tparam T The struct type.
 * 
 *  Note: We don't need tuple_to_struct since STL provide
 *  std::make_from_tuple which can construct an object from a tuple, and we can use it 
 *  together with struct_to_tuple to achieve the same effect as tuple_to_struct.
 * 
 *  For example, given a struct:
 *  struct MyStruct { int X; double Y; [[=cpp::refl::skip]] std::string Z; };
 *  MyStruct s{1, 3.14, "hello"};
 *  auto t = cpp::refl::struct_to_tuple(s);
 *  assert(t == std::make_tuple(1, 3.14));
 */
template <typename T>
constexpr auto struct_to_tuple(const T& t) 
{
    constexpr auto ctx = std::meta::access_context::current();
    constexpr auto members = define_static_array(nonstatic_data_members_of(^^T, ctx));
    constexpr auto [...Is] = indices_without_removed_member<T, cpp::refl::skip>();
    return std::make_tuple(t.[:members[Is]:]...);
}

// TODO: input range support
template <typename TupleLike, std::ranges::random_access_range Range>
constexpr TupleLike range_to_tuple(Range&& range)
{
    constexpr auto N = std::meta::tuple_size(^^TupleLike);
    constexpr auto [...idx] = std::make_index_sequence<N>();
    return TupleLike(std::forward_like<Range>(range[idx])...);
}

template <std::meta::info ClassInfo, std::meta::info FieldInfo>
struct field_handler
{
    static_assert(std::meta::is_class_type(ClassInfo) && std::meta::is_class_member(FieldInfo));

    using FieldType = typename [:type_of(FieldInfo):];

    static constexpr bool IsDefaultConstructible = std::is_default_constructible_v<FieldType>;

    static constexpr bool IsSkippable = cpp::refl::has_annotation(FieldInfo, cpp::refl::skip, cpp::refl::skip_deserialization);

    static constexpr bool IsUnnamedField = has_identifier(FieldInfo) == false;

    static_assert(!IsUnnamedField, "Unnamed field must be skippable since we have no way to initialize it.");

    template <typename Initializer>
    static constexpr std::optional<FieldType> init_value(Initializer initializer)
    {
        if constexpr (IsSkippable)
        {
            // Each member should be initialized in C++.
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


} // namespace cpp::refl

