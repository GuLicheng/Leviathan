#include <leviathan/math/int128.hpp>
#include <iostream>
#include <random>
#include <map>

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

    using T = std::map<int, int>::key_type;

    std::random_device rd;

    std::puts("OK1");

    return 0;
}




