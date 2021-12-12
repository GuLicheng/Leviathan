#ifndef __META_HPP__
#define __META_HPP__

#include <type_traits>
#include <functional> // reference_wrapper

namespace leviathan::meta
{
    template <bool IsConst, typename T>
    struct maybe_const : std::conditional<IsConst, const T, T> { };

    template <bool IsConst, typename T>
    struct maybe_const<IsConst, T *> : std::conditional<IsConst, const T *, T *> { };

    template <bool IsConst, typename T>
    struct maybe_const<IsConst, T &> : std::conditional<IsConst, const T &, T &> { };

    template <bool IsConst, typename T>
    using maybe_const_t = typename maybe_const<IsConst, T>::type;

    template <typename T, typename... Ts>
    struct should_be : std::disjunction<std::is_same<std::remove_cvref_t<T>, Ts>...> { };

    template <typename T, typename... Ts>
    constexpr bool should_be_v = should_be<T, Ts...>::value;

    template <typename T>
    struct referece_traits : std::type_identity<T> { };

    template <typename T>
    struct referece_traits<std::reference_wrapper<T>> : std::type_identity<T&> { };

}

#endif