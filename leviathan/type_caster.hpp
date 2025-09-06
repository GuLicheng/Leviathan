#pragma once

#include <leviathan/extc++/concepts.hpp>
#include <charconv>
#include <concepts>
#include <optional>
#include <expected>
#include <string_view>
#include <string>
#include <format>

namespace cpp
{

enum class cast_error
{
    ok,     
    type_error,   
};

enum class error_policy
{
    exception,   // -> throw std::exception
    expected,    // -> return std::expected<T, cast_error>
    optional,    // -> return std::optional<T>
};

template <typename Target, typename Source, error_policy Policy = error_policy::exception>
class type_caster;

// ------------------------------------ Specialized for some basic type ------------------------------------

// Type caster for arithmetic types from string_view using std::from_chars.
template <cpp::meta::arithmetic Arithmetic, error_policy Policy>
class type_caster<Arithmetic, std::string_view, Policy>
{
    static auto get_return()
    {
        if constexpr (Policy == error_policy::exception)
        {
            return Arithmetic{};
        }
        else if constexpr (Policy == error_policy::optional)
        {
            return std::optional<Arithmetic>{};
        }
        else // if constexpr (Policy == error_policy::expected)
        {
            return std::expected<Arithmetic, std::errc>{};
        }
    }

public:

    using result_type = decltype(get_return());

    template <typename... Args>
        requires (Policy == error_policy::optional)
    static constexpr std::optional<Arithmetic> operator()(std::string_view ctx, Args... args) noexcept
    {
        Arithmetic result;
        auto [ptr, ec] = std::from_chars(ctx.data(), ctx.data() + ctx.size(), result, args...);
        return ec == std::errc() && ptr == ctx.end() 
             ? std::make_optional(result)
             : std::nullopt;
    }

    template <typename... Args>
        requires (Policy == error_policy::expected)
    static constexpr std::expected<Arithmetic, std::errc> operator()(std::string_view ctx, Args... args) noexcept
    {
        Arithmetic result;
        auto [ptr, ec] = std::from_chars(ctx.data(), ctx.data() + ctx.size(), result, args...);
        return ec == std::errc() && ptr == ctx.end() 
             ? std::expected<Arithmetic, std::errc>(std::in_place, result)
             : std::expected<Arithmetic, std::errc>(std::unexpect, ec);   // FIXME: ec may not a good choice
    }


    template <typename... Args>
        requires (Policy == error_policy::exception)
    static constexpr Arithmetic operator()(std::string_view ctx, Args... args)
    {
        auto result = type_caster<Arithmetic, std::string_view, error_policy::optional>::operator()(ctx, args...);

        return result ? *result
                      : throw std::runtime_error(std::format("Failed to convert string '{}' to arithmetic type", ctx));
    }
};

// Type caster for arithmetic types from string using std::from_chars.
template <cpp::meta::arithmetic Arithmetic, error_policy Policy>
class type_caster<Arithmetic, std::string, Policy> : public type_caster<Arithmetic, std::string_view, Policy>
{
public:

    using result_type = typename type_caster<Arithmetic, std::string_view, Policy>::result_type;

    template <typename... Args>
    static constexpr result_type operator()(const std::string& ctx, Args... args)
    {
        return type_caster<Arithmetic, std::string_view, Policy>::operator()(std::string_view(ctx), args...);
    }
};

// Type caster for string types from arithmetic using std::format.
template <cpp::meta::arithmetic Arithmetic>
class type_caster<std::string, Arithmetic, error_policy::exception>
{
public:

    using result_type = std::string;

    template <typename... Args>
    static constexpr result_type operator()(Arithmetic value, std::format_string<Args...> fmt)
    {
        return std::vformat(fmt.get(), std::make_format_args(value));
    }

    static constexpr result_type operator()(Arithmetic value)
    {
        return std::format("{}", value);
    }
};

template <typename Enum, error_policy Policy>
class type_caster<Enum, Enum, Policy>
{
public:

    using result_type = Enum;

    static constexpr result_type operator()(Enum value)
    {
        return value;
    }
};

template <typename Target, error_policy Policy>
struct caster
{
    template <typename Source, typename... Args>
    static constexpr auto operator()(const Source& source, Args... args)
    {
        using Caster = type_caster<Target, Source, Policy>;

        if constexpr (meta::complete<Caster>)
        {
            return Caster::operator()(source, args...);
        }
        else if constexpr (std::is_same_v<std::string, Target>)
        {
            // We add a specialization for std::string temporary. I am not sure 
            // whether it will cause some errors. The people may only
            // add overloading for std::format_as or specialization for 
            // std::formatter. For above cases, our caster can work for std::string.
            return std::format("{}", source);
        }
        else 
        {
            static_assert(sizeof...(Args) == 0, "Invalid arguments for type_caster");
            return static_cast<Target>(source);
        }
    }

    // Specialized cast function for string literals.
    template <typename... Args>
    static constexpr auto operator()(const char* source, Args... args)
    {
        std::string_view sv(source);
        return operator()(sv, args...);
    }
};

template <typename Target>
inline constexpr auto cast = caster<Target, error_policy::exception>();

} // namespace cpp

