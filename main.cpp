#include <winnow/error.hpp>
#include <winnow/utils.hpp>
#include <winnow/result.hpp>
#include <print>

template <template <typename...> typename Container>
struct Example
{
    template <typename T>
    static constexpr auto DoSomething(T value) 
    {
        Container<T> container{value};
        return container;
    }
};

template <template <typename...> typename Container>
inline constexpr Example<Container> Instance;

int main()
{
    auto vec = Instance<std::vector>.DoSomething(40);



    std::println("vec = {}", vec);
}
