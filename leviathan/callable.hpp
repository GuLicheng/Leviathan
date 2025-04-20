#pragma once

#include <optional>
#include <any>
#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <ranges>
#include <algorithm>
#include <leviathan/extc++/concepts.hpp>
#include <leviathan/meta/function_traits.hpp>
#include <optional>

namespace cpp
{
    
namespace detail
{
    
template <typename ClassType, bool Const> // bool IsViolate ?
struct member_object_pointer : std::conditional<Const, const ClassType*, ClassType*>
{ };

template <typename T, typename... Args>
auto push_front_type(std::tuple<Args...>) -> std::tuple<T, Args...>;

// for member function pointer, the first arg should be obj*/const obj*
template <typename F>
struct function_args
{
private:
    using traits = cpp::meta::function_traits<F>;
    using arg_tuple = typename traits::args;
    using class_type = typename traits::class_type;
    static constexpr bool is_const = traits::attribute & 0x0001;
    static constexpr bool is_class_member_pointer = std::is_member_function_pointer_v<F>; // member_obj ?
    using member_object = typename member_object_pointer<class_type, is_const>::type;

public:

    using type = std::conditional_t<
        is_class_member_pointer,
        decltype(push_front_type<member_object>(std::declval<arg_tuple>())),
        arg_tuple
    >;
};

template <typename Fn>
struct invoker
{
    using traits = meta::function_traits<Fn>;

    static void apply(Fn fn, std::any arg, std::optional<std::any>& out)
    {
        using ArgType = typename function_args<Fn>::type;
        using R = typename traits::return_type;

        static_assert(std::is_same_v<R, std::remove_reference_t<R>>, "Return type should not be reference such as int&");

        auto& tp = std::any_cast<ArgType&>(arg);
        
        if constexpr (std::is_void_v<R>)
        {
            std::apply(std::move(fn), tp);
        }
        else
        {
            auto res = std::apply(std::move(fn), tp);
            out.emplace(std::move(res));
        }
    }
};

}  // namespace detail

class callable
{
    using function_type = std::function<void(std::any, std::optional<std::any>&)>;
    using key_type = std::string;
    using value_type = std::pair<key_type, function_type>;

public:

    callable() = default;

    template <typename Fn>
    void register_handler(std::string name, Fn fn)
    {
        if (std::ranges::contains(m_functions | std::views::keys, name))
        {
            throw std::invalid_argument("Name conflict");
        }

        m_functions.emplace_back(
            std::move(name),
            std::bind_front(&detail::invoker<Fn>::apply, std::move(fn))
        );
    }

    void remove_handler(std::string_view name)
    {
        std::erase_if(m_functions, [=](const auto& value) { return value.first == name; });
    }

    template <typename R, typename... Args>
    auto call(std::string_view name, Args&&... args)
    {
        auto it = std::ranges::find_if(m_functions, [=](const auto& value) { return value.first == name; });
    
        if (it == m_functions.end())
        {
            throw std::invalid_argument("Function not found");
        }

        std::optional<std::any> retval;
        auto tp = std::make_tuple((Args&&)args...);
        it->second(tp, std::ref(retval));
    
        if constexpr (!std::is_void_v<R>)
        {
            return std::move(std::any_cast<R>(*retval));
        }
    }

    // Remove all functions
    void clear() 
    {
        m_functions.clear();
    }

private:

    std::vector<value_type> m_functions;

};

} // namespace cpp

