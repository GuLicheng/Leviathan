#pragma once

#include <leviathan/extc++/concepts.hpp>
#include <charconv>
#include <concepts>
#include <optional>
#include <string_view>
#include <string>

namespace cpp
{

enum class error_code
{
    ok,        
};

enum class error_policy
{
    exception,   // -> throw std::exception
    expected,    // -> return std::expected<T, error_code>
    optional,    // -> return std::optional<T>
};

template <typename Target, typename Source, error_policy Policy = error_policy::exception>
class type_caster;

template <typename Target, error_policy Policy = error_policy::exception>
class simple_caster
{
public:

    template <typename Source>
    static constexpr auto operator()(const Source& source)
    {
        return type_caster<Target, Source, Policy>::operator()(source);
    }
};

template <typename Target, typename Source>
constexpr auto cast(const Source& source)
{
    using Caster = type_caster<Target, Source, error_policy::exception>;

    if constexpr (meta::complete<Caster>)
    {
        return Caster::operator()(source);
    }
    else 
    {
        return static_cast<Target>(source);
    }
}

// ------------------------------------ String To Arithmetic ------------------------------------
template <cpp::meta::arithmetic Arithmetic, error_policy Policy>
    requires (Policy == error_policy::exception || Policy == error_policy::optional)
class type_caster<Arithmetic, std::string_view, Policy>
{
public:

    using result_type = std::conditional_t<Policy == error_policy::exception, Arithmetic, std::optional<Arithmetic>>;

    template <typename... Args>
        requires (Policy == error_policy::optional)
    static constexpr result_type operator()(std::string_view ctx, Args... args)
    {
        Arithmetic result;
        auto [ptr, ec] = std::from_chars(ctx.data(), ctx.data() + ctx.size(), result, args...);
        return ec == std::errc() && ptr == ctx.end() 
             ? std::make_optional(result)
             : std::nullopt;
    }

    template <typename... Args>
        requires (Policy == error_policy::exception)
    static constexpr result_type operator()(std::string_view ctx, Args... args)
    {
        auto result = type_caster<Arithmetic, std::string_view, error_policy::optional>::operator()(ctx, args...);

        return result ? *result
                       : throw std::runtime_error(std::format("Failed to convert string '{}' to arithmetic type", ctx));
    }
};

template <typename Target, error_policy Policy>
class type_caster<Target, std::string, Policy> : public type_caster<Target, std::string_view, Policy>
{
public:

    using result_type = typename type_caster<Target, std::string_view, Policy>::result_type;

    template <typename... Args>
    static constexpr result_type operator()(const std::string& ctx, Args... args)
    {
        return type_caster<Target, std::string_view, Policy>::operator()(std::string_view(ctx), args...);
    }
};



} // namespace cpp

