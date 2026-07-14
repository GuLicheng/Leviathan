#pragma once

#include <leviathan/extc++/meta.hpp>

#include <algorithm>
#include <functional>
#include <ranges>

namespace cpp::derive
{

/**
 * @brief Allow a class or enum type to be formatted with std::format. 
 */
inline constexpr struct { } debug;

/**
 * @brief Allow a class or enum type to be hashed with std::hash.
 */
inline constexpr struct { } hash;

/**
 * @brief Allow a class or enum to be converted into another 
 * type, for example, json::value. Specialize the `cpp::type_caster` for the 
 * target type to implement the conversion.
 */
template <typename T> struct from_t { explicit from_t() = default; };
template <typename T> inline constexpr auto from = from_t<T>{};

/**
 * @brief Allow a class or enum to be converted from another 
 * type, for example, json::value. Specialize the `cpp::type_caster` for the 
 * source type to implement the conversion.
 */
template <typename T> struct into_t { explicit into_t() = default; };
template <typename T> inline constexpr auto into = into_t<T>{};

/**
 * @brief Allow an enum type to support operator| and operator|=, 
 * which is useful for bitmask operations.
 */
inline constexpr struct { } op_pipe;

/**
 * @brief Allow a class to be treated as a tuple-like type, 
 * which make it supported by std::tuple_size, std::tuple_element.
 * You must derive from `cpp::tuple_get_interface` to provide the get 
 * function for each field.
 */
inline constexpr struct { } tuple_like;

}  // namespace cpp::derive

namespace cpp::refl
{

struct annotation { };

// Rename field or classname, which will be used when serializing or 
// deserializing the field or class.
// struct SomeAnnotation { static constexpr std::string operator()(std::string); };
struct rename_annotation : annotation { };

// For a field, provide a default value for it, which will be 
// used when the field is not present in the input data.
// struct SomeAnnotation { static constexpr auto operator(); };
struct initializer_annotation : annotation { };

// Check if a field is valid according to the value_guard annotation, which will be
// used when initializing the field. The value_guard annotation should be a callable 
// type, which takes the field value as input and returns a boolean value 
// indicating whether the field is valid or not.
// struct SomeAnnotation { static constexpr bool operator()(const auto&); };
struct guard_annotation : annotation { };

// Any type inherit from serializer_annotation will be treated as a serializer, which means that
// when serializing the type, we will use the serializer to convert it into target type.
struct serializer_annotation : annotation { };

// Any type inherit from deserializer_annotation will be treated as a deserializer, which means that
// when deserializing the type, we will use the deserializer to convert it into target
// type. We do not require the result type of the deserializer to be the string type.
// You can deserializer it as any type you want, such as std::string, SomeBase64, etc.
struct deserializer_annotation : annotation { };

// Any class inherit from source_annotation will be treated as a range producer, 
// class SomeInterface { std::ranges::range<R> operator()(); }
struct source_annotation : annotation { };

struct alias_annotation : annotation { };

// Any field annotated with [[=skip]] will be ignored in code generation
// When initializing a struct from a tuple, the skipped fields will be 
// initialized with default value or default initializer.
inline constexpr struct { } skip;

inline constexpr struct { } skip_deserialization;

inline constexpr struct { } skip_serialization;

inline constexpr struct { } test;

// Any field annotated with [[=flatten]] will be treated as a flatten field, which means that
// when serializing the field, we will serialize its members instead of the field itself.
inline constexpr struct { } flatten;

// Any field annotated with [[=required]] will be treated as a required field, which means that
// when initializing the field, we will check if the field is present in the input data.
inline constexpr struct { } required;

// Any field annotated with [[=constructor]] will be treated as a constructor field, which means that
// when initializing the field, we will use the constructor to initialize it.
// There must be only one constructor field in a struct, and it must be a non-static data member.
inline constexpr struct { } constructor;

}  // namespace cpp::refl

namespace cpp::refl
{

template <typename Annotation, typename F>
struct callable : public Annotation
{
    F function;

    explicit constexpr callable(F function) : Annotation(), function(std::move(function)) {}

    template <typename Self, typename... Args>
    constexpr auto operator()(this Self&& self, Args&&... args) 
    {
        return std::invoke(((Self&&)self).function, (Args&&)args...);
    }
};

template <typename Annotation, typename F>
constexpr auto make_callable(F f) 
{
    return callable<Annotation, F>(std::move(f));
}

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

inline constexpr auto shortname = make_callable<rename_annotation>([](std::string field_name) static 
{
    // assert(!name.empty(), "Name cannot be empty");
    return '-' + std::string(field_name.begin(), field_name.begin() + 1);
});

inline constexpr auto longname = make_callable<rename_annotation>([](std::string field_name) static 
{
    // assert(!name.empty(), "Name cannot be empty");
    return "--" + std::string(field_name);
});

inline constexpr auto selfname = make_callable<rename_annotation>([](std::string field_name) static 
{
    return field_name;
});

inline constexpr auto lowercase = make_callable<rename_annotation>([](std::string field_name) static 
{
    return field_name | std::views::transform(::tolower) | std::ranges::to<std::string>();
});

inline constexpr auto uppercase = make_callable<rename_annotation>([](std::string field_name) static 
{
    return field_name | std::views::transform(::toupper) | std::ranges::to<std::string>();
});

inline constexpr auto rename = [](std::string_view new_name) static
{
    return make_callable<rename_annotation>([name=define_static_string(new_name)](auto&&...) {
        return std::string(name);
    });
};

// Follows functions in terms of implementation maybe incorrect
// FIXME: Rust clap-
inline constexpr auto camel_case = make_callable<rename_annotation>([](std::string field_name) static
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

inline constexpr auto pascal_case = make_callable<rename_annotation>([](std::string field_name) static
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

inline constexpr auto kebab_case = make_callable<rename_annotation>([](std::string field_name) static
{
    return field_name | std::views::transform([](char c) { return c == '_' ? '-' : c; }) | std::ranges::to<std::string>();
});

template <typename T>
struct function_array_annotation : initializer_annotation
{
    const T* data;

    size_t size;

    consteval function_array_annotation(std::initializer_list<T> init) : data(define_static_array(init).data()), size(init.size()) { }

    constexpr function_array_annotation(const T* data, size_t size) : data(data), size(size) { }

    // The range should be constructible from random_access_iterator, which is the case for most of the standard containers.
    template <std::ranges::range R>
    constexpr operator R() const { return R(data, data + size); }

    // We assume the value can get the value by invoke itself, so we return itself here
    // and try cast it to the target type in value.
    constexpr auto& operator()() const { return *this; }
};

inline constexpr struct 
{
    template <typename T>
    static constexpr auto operator()(const T& value) 
    {
        return make_callable<initializer_annotation>([x = *std::define_static_object(value)]() { return x; });
    }

    template <typename T>
    static constexpr auto operator()(std::initializer_list<T> values) 
    {
        return function_array_annotation<T>(values);
    }

} default_value;

inline constexpr auto choice = []<typename... Ts>(Ts&&... ts) 
{
    return make_callable<guard_annotation>([...ts=(Ts&&)ts](const auto& value) {
        return ((value == ts) || ...);
    });
};

inline constexpr auto range = []<typename Lower, typename Upper>(Lower lower, Upper upper) 
{
    return make_callable<guard_annotation>([lower, upper](const auto& value) {
        return value >= lower && value <= upper;
    });
};

inline constexpr auto alias = []<typename... Ts>(Ts&&... value) static
{
    return make_callable<alias_annotation>([...value=std::define_static_string(value)]() {
        return std::vector<std::string>{value...};
    });
};

} // namespace cpp::refl


namespace cpp::refl
{

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
            constexpr auto renames = define_static_array(select_annotations_with_type(FieldInfo, ^^rename_annotation)); 
            
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

    /**
     * @brief Extract the name of a member by its annotation. If multiple annotations are provided, 
     * the first annotation that can extract a name will be used.
     * 
     * @example
     *  struct MyStruct { int X; double [[=rename("Z")]] Y; };
     *  std::string name1 = identifier<^^MyStruct::X>(); // "X"
     *  std::string name2 = identifier<^^MyStruct::Y>(); // "Z"
     */
    static constexpr std::string identifier() 
    {
        auto name = std::string(identifier_of(FieldInfo));
        return identifier(std::move(name));
    }

    static constexpr std::vector<std::string> identifier_and_aliases() 
    {
        std::vector<std::string> names { identifier() };

        constexpr static auto aliases = define_static_array(select_annotations_with_type(FieldInfo, ^^alias_annotation));

        template for (constexpr auto info : aliases)
        {
            auto alias_names = std::invoke(extract<typename [:type_of(info):]>(info));
            
            // Unique
            std::ranges::copy_if(alias_names, std::back_inserter(names), [&names](const auto& alias_name) {
                return std::ranges::find(names, alias_name) == names.end();
            });
        }

        return names;
    }

    static constexpr auto default_value() 
    {
        using Type = typename [:type_of(FieldInfo):];

        std::optional<Type> value = std::nullopt;

        constexpr auto initializers = define_static_array(select_annotations_with_type(FieldInfo, ^^initializer_annotation));

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
    constexpr static auto members = std::define_static_array(all_nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()));
    constexpr auto [...indices] = std::make_index_sequence<members.size()>{};
    
    auto impl = [&]<size_t Idx>() {
        constexpr auto gurads = define_static_array(select_annotations_with_type(members[Idx], ^^cpp::refl::guard_annotation));
        constexpr auto [...guard_indices] = std::make_index_sequence<gurads.size()>{};
        return (... && std::invoke(extract<typename [:type_of(gurads[guard_indices]):]>(gurads[guard_indices]), x.[:members[Idx]:]));
    };

    return (... && impl.template operator()<indices>());
}

consteval std::vector<std::meta::info> no_skipped_fields(std::meta::info type, std::meta::access_context ctx)
{
    return nonstatic_data_members_of(type, ctx) 
         | std::views::filter(std::bind_back(cpp::refl::has_annotations, cpp::refl::skip))
         | std::ranges::to<std::vector>();
}

} // namespace cpp::refl

