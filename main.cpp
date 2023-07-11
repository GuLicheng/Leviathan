#include <array>
#include <utility>

enum class ApplicationEvents{E1, E2, EMAX};

template<ApplicationEvents>
void function(){}

template<std::size_t... N>
constexpr auto testing(std::index_sequence<N...> indecs)
{
    constexpr auto arrays = std::array<void(*)(void), indecs.size()>{+[]
    {

        constexpr auto value = static_cast<ApplicationEvents>(N);
        [](){function<value>();}();

    }...};
    return arrays;
}


int main()
{
    testing(std::make_index_sequence<100>{});
    return 0;
}