#pragma once

#include <leviathan/meta/function_traits.hpp>
#include <leviathan/meta/core.hpp>

#include <any>
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>
#include <string>
#include <string_view>
#include <functional>
#include <tuple>

namespace leviathan
{
    
template <typename T>
struct callable_result : std::unique_ptr<T> 
{
    using std::unique_ptr<T>::unique_ptr;
    using std::unique_ptr<T>::operator=;
};

template <>
struct callable_result<void> { }; 

namespace detail
{

template <typename ClassType, bool Const> // bool IsViolate ?
struct member_object_pointer : std::conditional<Const, const ClassType*, ClassType*>
{ };

// for member function pointer, the first arg should be obj*/const obj*
template <typename F>
struct function_args
{
private:
    using traits = leviathan::meta::function_traits<F>;
    using arg_tuple = typename traits::args;
    using class_type = typename traits::class_type;
    static constexpr bool is_const = traits::attribute & 0x0001;
    static constexpr bool is_class_member_pointer = std::is_member_function_pointer_v<F>; // member_obj ?

public:

    using type = std::conditional_t<
        is_class_member_pointer,
        typename leviathan::meta::push_front<arg_tuple, typename member_object_pointer<class_type, is_const>::type>::type, 
        arg_tuple
    >;


};

//////////////////////////////////////////////////////////////////////////////////////
//                          Helper function
//////////////////////////////////////////////////////////////////////////////////////
template <typename F, typename Tuple>
void apply_without_last_element(F&& f, Tuple&& t)
{
    auto impl = []<typename F2, size_t... Idx>(F2&& f, Tuple&& t, std::index_sequence<Idx...>)
    {
        return std::invoke((F2&&)f, std::get<Idx>(t)...);
    };

    // return type should not be reference such as int&
    using R = typename leviathan::meta::function_traits<F>::return_type;
    static_assert(std::is_same_v<R, std::remove_reference_t<R>>);

    constexpr auto sz = std::tuple_size_v<std::remove_cvref_t<Tuple>> - 1; // remove last 
    if constexpr (std::is_void_v<R>)
    {
        impl((F&&)f, (Tuple&&)t, std::make_index_sequence<sz>());
    }
    else
    {
        auto res = impl((F&&)f, (Tuple&&)t, std::make_index_sequence<sz>());
        std::get<sz>(t).reset(new R(std::move(res)));
    }
}

//////////////////////////////////////////////////////////////////////////////////////
//                           Invoker
//////////////////////////////////////////////////////////////////////////////////////
template <typename F>
struct invoker
{
    static void apply(F f, std::any arg)
    {
        using arg_type = typename function_args<F>::type;
        using R = typename leviathan::meta::function_traits<F>::return_type;
        using any_type = typename leviathan::meta::push_back<arg_type, callable_result<R>&>::type;
        // PrintTypeInfo(any_type);
        auto& tp = std::any_cast<any_type&>(arg);
        apply_without_last_element(std::move(f), tp);
    }
};

}

class callables
{
    using value_type = std::pair<std::string, std::function<void(std::any)>>;

public:

    auto find_by_name(std::string_view name)
    {
        return std::find_if(m_functions.begin(), m_functions.end(), [=](const auto& kv) { return kv.first == name; });
    }

    template <typename Fn>
    void register_handler(std::string name, Fn fn)
    {
        auto it = find_by_name(name);

        if (it != m_functions.end())
        {
            throw std::invalid_argument("function already existed");
        }
        m_functions.emplace_back(
            std::move(name), 
            std::bind_front(&detail::invoker<std::remove_cvref_t<Fn>>::apply, std::move(fn)));
    }

    void destroy_handler(std::string_view name)
    {
        std::erase_if(m_functions, [=](const auto& kv) { return kv.first == name; });
    }

    void destroy_all_handlers()
    {
        m_functions.clear();
    }

    template <typename R, typename... Args>
    callable_result<R> call_by_name(std::string_view name, Args&&... args)
    {
        auto it = find_by_name(name);

        if (it == m_functions.end())
        {
            throw std::invalid_argument("function dose not existed");
        }

        callable_result<R> retval;
        auto tp = std::make_tuple((Args&&)args..., std::ref(retval));
        it->second(tp);
        return retval;
    }

private:

    std::vector<value_type> m_functions;
};

} // namespace leviathan

