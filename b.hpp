#include <functional>
#include <tuple>
#include <type_traits>


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
    F m_func;
    std::tuple<BoundArgs...> m_bound_args;
};



