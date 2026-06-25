#pragma once

#include <leviathan/extc++/concepts.hpp>
#include <charconv>
#include <concepts>
#include <optional>
#include <expected>
#include <string_view>
#include <string>
#include <format>
#include <meta>

namespace cpp
{

// Source != Target
template <typename Source, typename Target> 
struct optional_caster;

// string -> arithmetic
template <cpp::meta::arithmetic Arithmetic>
struct optional_caster<std::string_view, Arithmetic>
{
    static constexpr std::optional<Arithmetic> operator()(std::string_view ctx)
    {
        Arithmetic result;
        auto [ptr, ec] = std::from_chars(ctx.data(), ctx.data() + ctx.size(), result);
        return ec == std::errc() && ptr == ctx.end() 
             ? std::make_optional(result)
             : std::nullopt;
    }
};

// arithmetic -> string
template <cpp::meta::arithmetic Arithmetic>
struct optional_caster<Arithmetic, std::string>
{
    // For some language like C#, the ToString method may also accept some formatting arguments, 
    // so we add a variadic template for format string and arguments.
    template <typename... Args>
    static constexpr std::optional<std::string> operator()(Arithmetic value, std::string_view fmt)
    {
        try
        {
            // The fmt maybe invalid, so we need to catch the exception and return std::nullopt in that case.
            std::string fmtstr = '{' + std::string(fmt) + '}';
            return std::make_optional(std::vformat(std::string_view(fmtstr), std::make_format_args(value)));
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    // We add another overload for the case when there is no format string, which simply uses the default formatting.
    template <typename... Args>
    static constexpr std::optional<std::string> operator()(Arithmetic value)
    {
        // The default formatting should not throw exceptions
        return std::make_optional(std::format("{}", value));
    }
};

// string_view -> bool
template <>
struct optional_caster<std::string_view, bool>
{
    static constexpr bool is_true(std::string_view ctx)
    {
        return ctx == "true" || ctx == "True" || ctx == "TRUE";
    }

    static constexpr bool is_false(std::string_view ctx)
    {
        return ctx == "false" || ctx == "False" || ctx == "FALSE";
    }

    static constexpr std::optional<bool> operator()(std::string_view ctx)
    {
        return is_true(ctx) ? std::make_optional(true) : 
               is_false(ctx) ? std::make_optional(false) : 
               std::nullopt;
    }
};

template <typename Target, typename Source, typename... Args>
concept castable = requires (Source source, Args&&... args) 
{
    { optional_caster<Source, Target>::operator()(source, (Args&&)args...) };
};

// caster
template <typename Target, typename Source, typename... Args>
constexpr std::optional<Target> cast_optional(const Source& source, Args&&... args)
{
    if constexpr (std::same_as<std::string, Source>)
    {
        std::string_view sv(source);
        return cast_optional<Target>(sv, (Args&&)args...);
    }
    if constexpr (std::same_as<Target, Source>)
    {
        static_assert(sizeof...(Args) == 0, "No additional arguments expected when Source and Target are the same");
        return std::make_optional(source);
    }
    else if constexpr (castable<Target, Source, Args...>)
    {
        return optional_caster<Source, Target>::operator()(source, (Args&&)args...);
    }
    else if constexpr (std::same_as<std::string, Target>)
    {
        // We add a specialization for std::string temporary. I am not sure 
        // whether it will cause some errors. The people may only
        // add overloading for std::format_as or specialization for 
        // std::formatter. For above cases, our caster can work for std::string.
        return std::make_optional(std::format("{}", source));
    }
    else
    {
        static_assert(sizeof...(Args) == 0, "Invalid arguments for type_caster");
        return std::make_optional(static_cast<Target>(source));
    }
}

template <typename Target>
struct caster
{
    template <typename Source, typename... Args>
    static constexpr auto operator()(const Source& source, Args&&... args)
    {
        constexpr auto SourceName = display_string_of(^^Source);
        constexpr auto TargetName = display_string_of(^^Target);

        auto result = cast_optional<Target>(source, (Args&&)args...);
        return result 
             ? std::move(*result) 
             : throw std::runtime_error(std::format("Failed to cast from {} to {}", SourceName, TargetName));
    }

    // Specialized cast function for string literals.
    template <typename... Args>
    static constexpr auto operator()(const char* source, Args&&... args)
    {
        std::string_view sv(source);
        return operator()(sv, (Args&&)args...);
    }
};

template <typename Target>
inline constexpr auto cast = caster<Target>();

} // namespace cpp

