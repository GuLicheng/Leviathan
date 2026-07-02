#pragma once

#include <meta>
#include <format>
#include <type_traits>
#include <string_view>
#include <stdexcept>
#include <string>
#include <vector>
#include <ranges>
#include <functional>
#include <leviathan//type_caster.hpp>
#include <leviathan/extc++/annotation.hpp>

namespace cpp::refl
{

namespace detail
{

template <typename T>
consteval std::vector<std::meta::info> all_bases_of_impl()
{
    constexpr auto ctx = std::meta::access_context::unchecked();
    constexpr auto bases = define_static_array(bases_of(^^T, ctx));
    constexpr auto [...indices] = std::make_index_sequence<bases.size()>();

    return std::views::concat(
        all_bases_of_impl<typename [:type_of(bases[indices]):]>()...,
        std::vector<std::meta::info>{ ^^T }
    ) | std::ranges::to<std::vector>();
}

}  // namespace detail

/**
 * @brief Get all base classes of a class, including indirect base classes. 
 * The result is sorted by the type order and contains no duplicate types.
 * 
 * @tparam T The class type to get the base classes of.
 * @return A vector of meta-information of the base classes of T.
 * 
 * @example
 *  struct Base1 {};
 *  struct Base2 : Base1 {};
 *  struct Derived : Base2 {};
 *  static_assert(all_bases_of<Derived>().size() == 2); // Base1 and Base2
 */
template <typename T>
consteval std::vector<std::meta::info> all_bases_of()
{
    auto bases = detail::all_bases_of_impl<T>();
    std::vector<std::meta::info> result;

    // The `dealias` is unnecessary here since we use T
    // as template parameter. Only std::meta::info should 
    // be considered for duplicate check.
    std::ranges::copy_if(bases, std::back_inserter(result), [&](auto info) {
        return !std::ranges::contains(result, info, std::meta::dealias);
    }, std::meta::dealias);

    return result;
}

/**
 * @brief Check if a type is derived from a template.
 * @tparam Type The type to check.
 * @tparam ClassTemplate The template to check against.
 * 
 * @example
 *  static_assert(cpp::refl::is_derived_from_template<^^std::vector<int>, ^^std::vector>()); // true
 */
template <std::meta::info Type, std::meta::info ClassTemplate>
consteval bool is_derived_from_template()
{
    constexpr auto bases = define_static_array(all_bases_of<typename [:Type:]>());
    constexpr auto [...indices] = std::make_index_sequence<bases.size()>{};
    auto check = [&]<size_t Idx>() {
        return has_template_arguments(bases[Idx]) && template_of(bases[Idx]) == dealias(ClassTemplate);
    };

    return (check.template operator()<indices>() || ...);
}

/**
 * @brief Get all parent levels of a type, including itself. The class
 * itself is declared and the namespace it belongs to are all considered as its parent levels. 
 * 
 * @example
 *  
 *  namespace A { struct B { struct C {}; }; }
 *  all_parents<^^A::B::C>() -> [^^A::B::C, ^^A::B, ^^A, ^^::]
 */
template <std::meta::info Info>
consteval std::vector<std::meta::info> all_parents()
{
    std::vector<std::meta::info> result;
    
    for (auto cur = dealias(Info); cur != ^^::; cur = parent_of(cur))
    {
        result.push_back(cur);
    }

    result.push_back(^^::);
    return result;
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
template <std::meta::info Type, std::meta::info... ClassTemplates>
consteval bool instance_of_template()
{
    constexpr auto type = dealias(Type);
    return has_template_arguments(type) 
        && ((template_of(type) == dealias(ClassTemplates)) || ...);
}

/**
 * @brief Check if the given annotation is present on the given info.
 * @param r Anything that can be reflected, such as class, field, base class, etc.
 * @param obj The annotation to check.
 * @return true if the annotation is present, false otherwise.
 * 
 * @example
 * struct SomeThing { [[=some_annotation]] int x; }
 * static_assert(has_annotation(^^SomeThing::x, some_annotation));
 * 
 * inline constexpr auto SomeInstance [[=some_annotation]] = SomeInstance{};
 * static_assert(has_annotation(^^SomeInstance, some_annotation));
 */
template <typename... Ts>
consteval bool has_annotation(std::meta::info r, const Ts&... objs) 
{
    return (... || std::ranges::contains(
        annotations_of_with_type(r, ^^Ts),
        std::meta::reflect_constant(objs),
        std::meta::constant_of
    ));
}

/**
 * @brief Get the valid member indices of a class, excluding the members with [[=Annotations]] annotation.
 * @tparam T The class type.
 * 
 * @example
 *  struct MyStruct { int X; [[=Annotations]] double Y; char Z; };
 *  The valid member indices of MyStruct are 0 and 2, while index 1 is skipped due to the annotation.
 */
template <typename T, auto... Annotations>
consteval std::meta::info remove_skipped_member()
{
    std::vector args { ^^size_t };
    constexpr auto ctx = std::meta::access_context::unchecked();
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
using indices_without_removed_member = typename [:remove_skipped_member<T, Annotations...>():];

template <typename T>
consteval std::vector<std::meta::info> all_nsdm_unchecked()
{
    constexpr auto ctx = std::meta::access_context::unchecked();
    constexpr auto bases =  define_static_array(bases_of(^^T, ctx));
    constexpr auto [...base_indices] = std::make_index_sequence<bases.size()>{};
    constexpr auto [...indices] = cpp::refl::indices_without_removed_member<T, cpp::refl::skip>();
    constexpr auto members = define_static_array(nonstatic_data_members_of(^^T, ctx));

    return std::views::concat(
        all_nsdm_unchecked<typename [:type_of(bases[base_indices]):]>()...,
        std::vector<std::meta::info>{ members[indices]... }
    ) | std::ranges::to<std::vector>();
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
 *  s.[:member_number<MyStruct>(0):] = 1;
 *  s.[:member_number<MyStruct>(1):] = 3.14;
 *  assert(s.X == 1 && s.Y == 3.14);
 */
template <typename T>
consteval std::meta::info member_number(size_t N)
{
    constexpr auto ctx = std::meta::access_context::unchecked();
    constexpr auto [...indices] = indices_without_removed_member<T, skip>();
    constexpr auto sarray[] = { indices... };
    return std::meta::nonstatic_data_members_of(^^T, ctx)[sarray[N]];
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
 *  s.[:member_named<MyStruct>("X"):] = 1;
 *  s.[:member_named<MyStruct>("Y"):] = 3.14;
 *  assert(s.X == 1 && s.Y == 3.14);
 *  assert(s.[:member_named<MyStruct>("ReturnConstant"):]() == 42);
 */
template <typename T>
consteval std::meta::info member_named(const char* name)
{
    auto ctx = std::meta::access_context::unchecked();
    for (std::meta::info field : members_of(^^T, ctx))
        if (has_identifier(field) && identifier_of(field) == name)
            return field;
    // throw std::runtime_error(std::format("No member named {} in type {}", name, identifier_of(^^T)));
    throw std::runtime_error("No member named " + std::string(name) + " in type " + std::string(identifier_of(^^T)));
}

namespace detail
{

static consteval bool has_modify_identifier(std::meta::info anno)
{
    // The anno must be an instance.
    return has_annotation(type_of(anno), modify_identifier);
}

template <std::meta::info Info>
struct extract_name_by_annotation_impl
{
    static constexpr std::string operator()(std::string name)
    {
        if constexpr (Info == ^^::)
        {
            return name;
        }
        else 
        {
            template for (constexpr auto anno : define_static_array(annotations_of(Info)))
                if constexpr (has_modify_identifier(anno))
                    return std::invoke(extract<typename [:type_of(anno):]>(anno), name);
            return extract_name_by_annotation_impl<parent_of(Info)>()(name);
        }
    } 
};

}  // namespace detail

/**
 * @brief Extract the name of a member by its annotation. If multiple annotations are provided, 
 * the first annotation that can extract a name will be used.
 * 
 * @tparam Info1 The meta-information of the member to extract the name from.
 * @tparam Infos The meta-information of the annotations to use for extracting the name.
 * 
 * @example
 *  struct MyStruct { int X; double [[=rename("Z")]] Y; };
 *  std::string name1 = extract_name_by_annotation<^^MyStruct::X>(); // "X"
 *  std::string name2 = extract_name_by_annotation<^^MyStruct::Y>(); // "Z"
 */
template <std::meta::info Info>
constexpr std::string extract_name_by_annotation()
{
    constexpr auto name = identifier_of(Info);
    return detail::extract_name_by_annotation_impl<Info>()(std::string(name));
}

template <std::meta::info FieldInfo>
struct field_handler;

/**
 * @brief Construct an object of type T by initializing its fields with the provided initializer.
 * @param T The type of the object to construct. Must be a class type.
 * @param Initializer A callable that takes a reference to an optional field value 
 *  and the field name, then initializes the field value if possible.
 *  
 * @example
 *  struct SomeInitializer {
 *      template <typename T, typename Caster> 
 *      void operator()(std::optional<T>& value, std::string name, Caster caster) {
 *          // Implementation here such as:
 *          auto result = GetValueByName(name);
 *          // Is cast failed, we can throw exception or just leave the value as std::nullopt.
 *          // However, if the value is std::nullopt, we will try to use the default value of the field if it has one.
 *          caster(value, result);
 *      }    
 *  };
 */
template <typename T, typename Resolver>
constexpr T construct_struct(Resolver resolver)
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
        cpp::refl::construct_struct<typename [:type_of(bases[base_indices]):]>(std::ref(resolver))...,
        cpp::refl::field_handler<members[indices]>()(std::ref(resolver))...
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
 * @example
 *  struct MyStruct { int X; double Y; [[=cpp::refl::skip]] std::string Z; };
 *  MyStruct s{1, 3.14, "hello"};
 *  auto t = cpp::refl::struct_to_tuple(s);
 *  assert(t == std::make_tuple(1, 3.14));
 */
template <typename T>
constexpr auto struct_to_tuple(const T& t) 
{
    constexpr auto members = define_static_array(all_nsdm_unchecked<T>());
    constexpr auto [...Is] = indices_without_removed_member<T, cpp::refl::skip>();
    return std::make_tuple(t.[:members[Is]:]...);
}

// TODO: input range support
// template <typename TupleLike, std::ranges::random_access_range Range>
// constexpr TupleLike range_to_tuple(Range&& range)
// {
//     constexpr auto N = std::meta::tuple_size(^^TupleLike);
//     constexpr auto [...idx] = std::make_index_sequence<N>();
//     return TupleLike(std::forward_like<Range>(range[idx])...);
// }

template <std::meta::info FieldInfo>
class field_handler
{
    static_assert(std::meta::is_class_member(FieldInfo));

    using FieldType = typename [:type_of(FieldInfo):];

    static constexpr bool IsDefaultConstructible = std::is_default_constructible_v<FieldType>;

    static constexpr bool IsSkippable = has_annotation(FieldInfo, skip);

    static constexpr bool IsUnnamedField = has_identifier(FieldInfo) == false;

    static_assert(!IsUnnamedField, "Unnamed field must be skippable since we have no way to initialize it.");

    static constexpr auto caster_adaptor = [](auto& opt, const auto& value)
    {
        opt.emplace(cpp::cast<FieldType>(value));
    };

    static consteval std::meta::info extract_serializer()
    {
        template for (constexpr auto anno : define_static_array(annotations_of(FieldInfo)))
            if constexpr (has_annotation(type_of(anno), serializer))
                return anno;
        return ^^caster_adaptor;
    }

    template <typename Resolver>
    static constexpr std::optional<FieldType> init_value(Resolver resolver)
    {
        if constexpr (IsSkippable)
        {
            // Each member should be initialized in C++.
            return default_value();
        }
        else
        {
            // Get field name
            auto name = extract_name_by_annotation<FieldInfo>();
            std::optional<FieldType> value = std::nullopt;

            // Try init current field with resolver
            constexpr auto serializer_info = extract_serializer();

            std::invoke(resolver, value, name, extract<typename [:type_of(serializer_info):]>(serializer_info));

            // Try init current field with annotations
            return value ? value : default_value();
        }
    }

    static constexpr std::optional<FieldType> default_value() 
    {
        template for (constexpr auto anno : define_static_array(annotations_of(FieldInfo)))
            if constexpr (has_annotation(type_of(anno), initializer))
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
            if constexpr (has_annotation(type_of(anno), value_guard))
                if (!std::invoke(extract<typename [:type_of(anno):]>(anno), value))
                    return false;
        return true;
    }

public:

    template <typename Resolver>
    static constexpr FieldType operator()(Resolver resolver)
    {
        auto value = init_value(resolver);
        return value.has_value() && is_valid(*value) 
             ? std::move(*value) 
             : throw std::runtime_error(std::format("Field {} is missing or invalid", display_string_of(FieldInfo)));
    }
};

/**
 * @brief Get all annotations of a type that have a specific type annotation.
 * @param info The meta-information of the type to get annotations from.
 * @param x The type annotation to filter annotations by.
 * 
 * @example
 *  inline constexpr struct { } serializer;
 *  struct [[=serializer]] SomeCallable { auto operator()(auto x); };
 *  struct MyStruct { [[=SomeCallable()]] int X; double Y; }; 
 *  auto vec = annotations_with_type_annotation(^^MyStruct::X, serializer);
 *  vec[0] -> instance of SomeCallable
 */
template <typename... Ts>
consteval std::vector<std::meta::info> select_annotations(std::meta::info info, const Ts&... xs) 
{
    return annotations_of(info) | std::views::filter([&](std::meta::info anno) {
        return has_annotation(type_of(anno), xs...);
    }) | std::ranges::to<std::vector>();
}

/**
 * @brief Select first info which satisfies the given annotation type, or return the default info if none found.
 * @param default_info The default info to return if no matching annotation is found.
 * @param info The meta-information to search for annotations.
 * @param xs The annotation types to search for.
 * 
 * @example
 */
template <typename... Ts>
consteval std::meta::info select_annotation(std::meta::info default_info, std::meta::info info, const Ts&... xs) 
{
    auto annos = select_annotations(info, xs...);
    return annos.size() > 0 ? annos[0] : default_info;
}

template <std::meta::info FieldInfo>
class handle
{
    template <std::meta::info>
    friend class handle;

    static constexpr std::string identifier(std::string name)
    {
        if constexpr (FieldInfo == ^^::)
        {
            return name;
        }
        else
        {
            constexpr auto renames = define_static_array(select_annotations(FieldInfo, modify_identifier)); 
            
            if constexpr (renames.size() > 0)
            {
                return std::invoke(extract<typename [:type_of(renames[0]):]>(renames[0]), name);
            }
            else
            {
                return handle<parent_of(FieldInfo)>::identifier(std::move(name));
            }
        }
    }

public:

    static constexpr std::string identifier() 
    {
        auto name = std::string(identifier_of(FieldInfo));
        return identifier(std::move(name));
    }

    static constexpr auto default_value() 
    {
        using Type = typename [:type_of(FieldInfo):];

        std::optional<Type> value = std::nullopt;

        constexpr auto initializers = define_static_array(select_annotations(FieldInfo, initializer));

        if constexpr (initializers.size() > 0)
        {
            value.emplace(std::invoke(extract<typename [:type_of(initializers[0]):]>(initializers[0])));
        }
        else if constexpr (std::is_default_constructible_v<Type>)
        {
            value.emplace();
        }

        return value;
    }

};

/**
 * @brief Check if all fields of a struct are valid according to their value_guard annotations.
 * @param x The object to check.
 * 
 * @example
 *  struct MyStruct { [[=cpp::refl::guard([](int x) { return x >= 0; })]] int X; };
 *  MyStruct s{42};
 *  assert(check_field(s)); // true
 *  MyStruct s2{-1};
 *  assert(!check_field(s2)); // false
 */
template <typename T>
constexpr bool check_field(const T& x)
{
    constexpr auto members = std::define_static_array(cpp::refl::all_nsdm_unchecked<T>());
    constexpr auto [...indices] = std::make_index_sequence<members.size()>{};
    
    auto impl = [&]<size_t Idx>() {
        constexpr auto gurads = define_static_array(select_annotations(members[Idx], cpp::refl::value_guard));
        constexpr auto [...guard_indices] = std::make_index_sequence<gurads.size()>{};
        return (... && std::invoke(extract<typename [:type_of(gurads[guard_indices]):]>(gurads[guard_indices]), x.[:members[Idx]:]));
    };

    return (... && impl.template operator()<indices>());
}


} // namespace cpp::refl

/*
template <typename T>
struct type
{
    static constexpr void show_all_members()
    {
        constexpr static auto members = define_static_array(cpp::refl::all_nsdm_unchecked<T>());
        template for (constexpr auto member : members)
        {
            std::print("Member: {}\n", has_identifier(member) ? identifier_of(member) : "<unnamed>");
        }
    }
};

*/