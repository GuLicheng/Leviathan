#include <leviathan/math/int128.hpp>
#include <iostream>
#include <random>


struct Double
{
    double value;

    constexpr explicit operator double(this Double x)
    {
        return x.value;
    }

};

int main(int argc, char const *argv[])
{
    Double d = { .value = 0.1 };

    auto dd = static_cast<double>(d);

    std::cout << dd << '\n';

    std::puts("OK1");

    return 0;
}




