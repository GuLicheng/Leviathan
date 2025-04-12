#pragma once

#include <functional>
#include <tuple>
#include <type_traits>

namespace cpp
{
    template <typename F, typename... BoundArgs>
    class bind_back_fn
    {
    public:

        static_assert(std::is_move_constructible_v<F>);
        static_assert((std::is_move_constructible_v<BoundArgs> && ...));

        constexpr bind_back_fn(const bind_back_fn&) = default;
        constexpr bind_back_fn(bind_back_fn&&) = default;
        constexpr bind_back_fn& operator=(const bind_back_fn&) = default;
        constexpr bind_back_fn& operator=(bind_back_fn&&) = default;
        constexpr ~bind_back_fn() = default;

        template <typename F2, typename... BoundArgs2>
        explicit constexpr bind_back_fn(int, F2&& f2, BoundArgs2&&... args2) 
            : m_func((F2&&)f2), m_bound_args((BoundArgs2&&)args2...)
        {
            static_assert(sizeof...(BoundArgs2) == sizeof...(BoundArgs));
        }


    private:

        using bound_indices = std::index_sequence_for<BoundArgs...>;


        template <typename Tp, size_t... Indices, typename... CallArgs>
        static constexpr decltype(auto)
        _S_call(Tp&& g, std::index_sequence<Indices...>, CallArgs&&... call_args)
        {
            return std::invoke(std::forward<Tp>(g).m_func,
                std::forward<CallArgs>(call_args)...,
                std::get<Indices>(std::forward<Tp>(g).m_bound_args)...);
        }

    public:

        template <typename... CallArgs>
        constexpr std::invoke_result_t<F&, BoundArgs&..., CallArgs...>
        operator()(CallArgs&&... call_args) &
        noexcept(std::is_nothrow_invocable_v<F&, BoundArgs&..., CallArgs...>)
        {
            return _S_call(*this, bound_indices(), 
                std::forward<CallArgs>(call_args)...);
        }

        template <typename... CallArgs>
        constexpr std::invoke_result_t<const F&, const BoundArgs&..., CallArgs...>
        operator()(CallArgs&&... call_args) const&
        noexcept(std::is_nothrow_invocable_v<const F&, const BoundArgs&..., CallArgs...>)
        {
            return _S_call(*this, bound_indices(), 
                std::forward<CallArgs>(call_args)...);
        }

        template <typename... CallArgs>
        constexpr std::invoke_result_t<F, BoundArgs..., CallArgs...>
        operator()(CallArgs&&... call_args) &&
        noexcept(std::is_nothrow_invocable_v<F, BoundArgs..., CallArgs...>)
        {
            return _S_call(std::move(*this), bound_indices(), 
                std::forward<CallArgs>(call_args)...);
        }

        template <typename... CallArgs>
        constexpr std::invoke_result_t<const F, const BoundArgs..., CallArgs...>
        operator()(CallArgs&&... call_args) const&&
        noexcept(std::is_nothrow_invocable_v<const F, const BoundArgs..., CallArgs...>)
        {
            return _S_call(std::move(*this), bound_indices(), 
                std::forward<CallArgs>(call_args)...);
        }

    private:
        F m_func;
        std::tuple<BoundArgs...> m_bound_args;
    };

    template<typename F, typename... Args>
    using bind_back_t = bind_back_fn<std::decay_t<F>, std::decay_t<Args>...>;


    template<typename F, typename... Args>
    constexpr bind_back_t<F, Args...>
    bind_back(F&& fn, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<bind_back_t<F, Args...>,
                    int, F, Args...>)
    {
        return bind_back_t<F, Args...>(0, std::forward<F>(fn),
                        std::forward<Args>(args)...);
    }

}


/*
void test_simple_function_with_two_arguments()
{
    auto minus = [](int a, int b) { return a - b; };
    auto minus_5 = cpp::bind_back(minus, 5);
    assert(minus_5(10) == 5);
}

int main(int argc, char const *argv[])
{
    test_simple_function_with_two_arguments();
    return 0;
}

*/

