#pragma once

#include <leviathan/extc++/concepts.hpp>
#include <charconv>
#include <concepts>
#include <optional>
#include <string_view>
#include <string>

namespace cpp
{

enum class error_policy
{
    exception,   // -> throw std::exception
    expected,    // -> return std::expected<T, E>
    optional,    // -> return std::optional<T>
};

template <typename Target, typename Source, error_policy Policy = error_policy::exception>
class type_caster;

template <cpp::meta::arithmetic Arithmetic>
class type_caster<Arithmetic, std::string_view, error_policy::optional>
{
public:

    using result_type = std::optional<Arithmetic>;

    template <typename... Args>
    static constexpr result_type operator()(std::string_view ctx, Args... args)
    {
        Arithmetic result;
        auto [ptr, ec] = std::from_chars(ctx.data(), ctx.data() + ctx.size(), result, args...);
        return ec == std::errc() && ptr == ctx.end() 
             ? std::make_optional(result)
             : std::nullopt;
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

