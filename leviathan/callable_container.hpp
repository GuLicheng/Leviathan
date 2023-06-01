#pragma once

//////////////////////////////////////////////////////////////////////////////////////
// Header files
//////////////////////////////////////////////////////////////////////////////////////

#include <leviathan/meta/core.hpp>
#include <leviathan/string/opt.hpp>

#include <memory>
#include <any>
#include <functional>
#include <unordered_map>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////////////
//                      Result Type
//////////////////////////////////////////////////////////////////////////////////////
// std::unique_ptr does not support void
template <typename T>
class unique_wrapper : public std::unique_ptr<T> 
{
public:
    using std::unique_ptr<T>::unique_ptr;
    using std::unique_ptr<T>::operator=;
};

template <>
class unique_wrapper<void> { };


//////////////////////////////////////////////////////////////////////////////////////
//                          Helper meta
//////////////////////////////////////////////////////////////////////////////////////

template <typename ClassType, bool IsConst> // bool IsViolate ?
struct member_object_pointer : std::conditional<IsConst, const ClassType*, ClassType*>
{ };


// for member function pointer, the first arg should be obj*/const obj*
template <typename F>
struct function_args
{
private:
    using traits = leviathan::meta::function_traits<F>;
    using arg_tuple = typename traits::args;
    using class_type = typename traits::class_type;
    constexpr static bool is_const = traits::attribute & 0x0001;
    constexpr static bool is_class_member_pointer = std::is_member_function_pointer_v<F>; // member_obj ?

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
        using any_type = typename leviathan::meta::push_back<arg_type, unique_wrapper<R>&>::type;
        // PrintTypeInfo(any_type);
        auto& tp = std::any_cast<any_type&>(arg);
        apply_without_last_element(std::move(f), tp);
    }
};


//////////////////////////////////////////////////////////////////////////////////////
//                          Callable Container
//////////////////////////////////////////////////////////////////////////////////////
struct callable_container
{
    template <typename F>
    void register_handler(std::string name, F&& f)
    {
        if (m_maps.count(name))
            throw std::invalid_argument("function already existed");
        m_maps[std::move(name)] = std::bind_front(&invoker<std::remove_cvref_t<F>>::apply, std::move(f));
    }

    void destroy_handler(std::string_view name)
    {
        // https://en.cppreference.com/w/cpp/container/unordered_map/erase (4) C++23 
        auto iter = m_maps.find(name);
        if (iter != m_maps.end())
            m_maps.erase(iter);
    }

    void destroy_all_handlers()
    {
        m_maps.clear();
    }

    template <typename R, typename... Args>
    unique_wrapper<R> call_by_name(std::string_view name, Args&&... args)
    {
        auto iter = m_maps.find(name);
        if (iter == m_maps.end())
            throw std::invalid_argument("function dose not existed");
        unique_wrapper<R> ret;
        auto tp = std::make_tuple((Args&&)args..., std::ref(ret));
        // PrintTypeCategory(tp);
        iter->second(tp);
        return ret;
    }

private:
    std::unordered_map<
        std::string, 
        std::function<void(std::any)>, 
        leviathan::string_hash, 
        leviathan::string_key_equal> m_maps;
};

