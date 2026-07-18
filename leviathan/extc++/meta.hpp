#pragma once

#include <leviathan/extc++/ranges.hpp>

#include <meta>
#include <functional>
#include <algorithm>

namespace cpp::refl
{

/**
 * @brief Get all base classes of a class, including indirect base classes. 
 * The result is sorted by the type order and contains no duplicate types.
 * 
 * @param info The meta-information of the class to get the base classes of.
 * 
 * @example
 *  struct Base1 {};
 *  struct Base2 : Base1 {};
 *  struct Derived : Base2 {};
 *  static_assert(all_bases_of(^^Derived) == {^^Derived, ^^Base2, ^^Base1});
 */
consteval std::vector<std::meta::info> all_bases_of(std::meta::info info)
{
    std::vector results { dealias(info) };

    // The info from bases_of is a base class specifier, we need to get the type of it.
    std::ranges::copy_if(
        bases_of(info, std::meta::access_context::unchecked())
        | cpp::views::compose(std::meta::type_of, all_bases_of)
        | std::views::join,
        std::back_inserter(results),
        [&](auto info) { return !std::ranges::contains(results, info, std::meta::dealias); },
        std::meta::dealias
    );

    return results;
}

/**
 * @brief std::is_derived_from for meta::info. Check if a class is derived from another class.
 * @param derived The meta-information of the derived class.
 * @param base The meta-information of the base class.
 * 
 * @example
 *  struct Base1 {};
 *  struct Derived : Base1 {};
 *  static_assert(is_derived_from(^^Derived, ^^Base1)); // true
 */
consteval bool is_derived_from(std::meta::info derived, std::meta::info base)
{
    return std::ranges::contains(all_bases_of(derived), dealias(base), std::meta::dealias);
}

/**
 * @brief Check if a type is an instance of a template.
 * @tparam Type The type to check.
 * @tparam ClassTemplates The template to check against.
 * 
 * @example
 *  static_assert(cpp::refl::instance_of_template<^^std::vector<int>, ^^std::vector>()); // true
 *  static_assert(cpp::refl::instance_of_template<^^std::tuple<int, int>, ^^std::tuple>()); // true
 */
consteval bool instance_of_template(std::meta::info info, std::meta::info template_info)
{
    auto type = dealias(info);
    return has_template_arguments(type) 
        && (template_of(type) == dealias(template_info));
}

/**
 * @brief Check if a type is derived from a template.
 * @param type The meta-information of the type to check.
 * @param template_info The meta-information of the template to check against.
 * 
 * @example
 *  static_assert(cpp::refl::is_derived_from_template(^^std::vector<int>, ^^std::vector)); // true
 */
consteval bool is_derived_from_template(std::meta::info type, std::meta::info template_info)
{
    return std::ranges::any_of(all_bases_of(type), [=](auto info) {
        return instance_of_template(info, template_info);
    });
}

/**
 * @brief Get all parent levels of a type, including itself. The class
 * itself is declared and the namespace it belongs to are all considered as its parent levels. 
 * 
 * @param info The meta-information of the type to get the parent levels of.
 * 
 * @example
 *  namespace A { struct B { struct C {}; }; }
 *  all_parents(^^A::B::C) -> [^^A::B::C, ^^A::B, ^^A, ^^::]
 */
consteval std::vector<std::meta::info> all_parents(std::meta::info info)
{
    return info == ^^:: 
         ? std::vector{ info } 
         : std::views::concat(std::views::single(info), all_parents(parent_of(info))) | std::ranges::to<std::vector>();
}

// Maybe callable object is better than just function.
// See std.ranges
inline constexpr struct
{   
    template <typename... Ts>
    static consteval bool operator()(std::meta::info r, const Ts&... objs)
    {
        return (... ||std::ranges::contains(
            annotations_of_with_type(r, ^^Ts),
            std::meta::reflect_constant(objs),
            std::meta::constant_of
        ));
    }
} has_annotations;

/**
 * @brief Get all non-static data members of a class, including those inherited from base classes.
 * @param info The meta-information of the class to get the non-static data members of.
 * @param ctx The access context to use when accessing the members.
 */
consteval std::vector<std::meta::info> all_nonstatic_data_members_of(std::meta::info info, std::meta::access_context ctx)
{
    return all_bases_of(info) 
         | cpp::views::transform_join(std::bind_back(std::meta::nonstatic_data_members_of, ctx))
         | std::ranges::to<std::vector>();
}

/**
 * @brief Get all annotations of a type that have a specific type annotation.
 *  Different from std::meta::annotations_of_with_type, this function will return 
 *  the annotations that are derived from the given type annotation, not just the exact type.
 * @param info The meta-information of the type to get annotations from.
 * @param type_or_template The meta-information of the type or template to filter annotations by.
 */
consteval std::vector<std::meta::info> select_annotations_with_type(std::meta::info info, std::meta::info type_or_template)
{ 
    auto instance_of = std::views::filter([=](std::meta::info anno) { 
        auto anno_type = type_of(anno);
        return is_template(type_or_template) 
             ? is_derived_from_template(anno_type, type_or_template) 
             : is_derived_from(anno_type, type_or_template); 
    });

    return annotations_of(info) 
         | instance_of
         | std::ranges::to<std::vector>();
}

consteval std::meta::info select_annotation_with_type(std::meta::info default_info, std::meta::info info, std::meta::info type_or_template)
{
    auto annotations = select_annotations_with_type(info, type_or_template);
    return annotations.size() > 0 ? annotations[0] : default_info;
}

/**
 * @brief Get the N-th member of a class by its declaration order.
 * @tparam T The class type.
 * @param N The index of the member, starting from 0.
 * @return The meta-information of the N-th member.
 * 
 * @example
 *  struct MyStruct { int X; double Y; };
 *  MyStruct s;
 *  s.[:member_number(^^MyStruct, 0):] = 1;
 *  s.[:member_number(^^MyStruct, 1):] = 3.14;
 *  assert(s.X == 1 && s.Y == 3.14);
 */
consteval std::meta::info member_number(std::meta::info type, size_t N)
{
    return all_nonstatic_data_members_of(type, std::meta::access_context::unchecked())[N];
}

/**
 * @brief Get the member of a class by its name. We use unchecked access context here 
 *        since we want to allow access to private members, and we will 
 *        check the access permission by ourselves.
 * 
 * @tparam T The class type.
 * @param name The name of the member.
 * @return The meta-information of the member with the given name.
 * 
 * @example
 *  struct MyStruct { 
 *      int X; 
 *      double Y; 
 *      int ReturnConstant() const { return 42; } 
 *  };
 * 
 *  MyStruct s;
 *  s.[:member_named(^^MyStruct, "X"):] = 1;
 *  s.[:member_named(^^MyStruct, "Y"):] = 3.14;
 *  assert(s.X == 1 && s.Y == 3.14);
 *  assert(s.[:member_named(^^MyStruct, "ReturnConstant"):]() == 42);
 */
consteval std::meta::info member_named(std::meta::info type, const char* name)
{
    auto members = members_of(type, std::meta::access_context::unchecked());
    auto same_as_name = [=](auto member) { return has_identifier(member) && identifier_of(member) == name; };
    return *std::ranges::find_if(members, same_as_name);
}

/**
 * @brief Convert a struct to a tuple by its members.
 * @tparam T The struct type.
 * 
 *  Note: We don't need tuple_to_struct since STL provide
 *  std::make_from_tuple which can construct an object from a tuple, and we can use it 
 *  together with struct_to_tuple to achieve the same effect as tuple_to_struct.
 * 
 * @example
 *  struct MyStruct { int X; double Y; std::string Z; };
 *  MyStruct s{1, 3.14, "hello"};
 *  auto t = cpp::refl::struct_to_tuple(s);
 *  assert(t == std::make_tuple(1, 3.14, "hello"));
 */
inline constexpr struct 
{
    template <typename T>
    static constexpr auto operator()(const T& t) 
    {
        constexpr auto members = define_static_array(all_nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()));
        constexpr auto [...indices] = std::make_index_sequence<members.size()>{};
        return std::make_tuple(t.[:members[indices]:]...);
    }
} struct_to_tuple;

} // namespace cpp::refl

