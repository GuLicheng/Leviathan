#include <iostream>
#include <functional>

template <typename Derived>
struct AdaptorClosure
{
    template<typename L, typename R>
    friend constexpr auto operator|(L&& lhs, R&& rhs);

    template<typename Self, typename Other>
	friend constexpr auto operator|(Other&& lhs, Self&& rhs);
};

template <typename L, typename R>
struct Pipe : AdaptorClosure<Pipe<L, R>>
{
    [[no_unique_address]] L lhs;
    [[no_unique_address]] R rhs;

    template <typename T, typename U>
    constexpr Pipe(T&& lhs, U&& rhs) : lhs(std::forward<T>(lhs)), rhs(std::forward<U>(rhs)) {}

    template <typename Self, typename Other>
    constexpr auto operator()(this Self&& self, Other&& other) const
    {
        return std::invoke(
            std::forward_like<Self>(self.rhs),
            std::invoke(
                std::forward_like<Self>(self.lhs),
                std::forward<Other>(other)
            )   
        );
    }

};


int main(int argc, char const *argv[])
{
    
    return 0;
}
