#include <leviathan/print.hpp>
#include <leviathan/utils/controllable_value.hpp>
#include <vector>


int main(int argc, char const *argv[])
{
    std::vector vec = { 1, 2, 3 };

    ::println("{}", vec);

    std::string_view sv = "";

    return 0;
}
// 1111111111111111111111111111111111111111111111111111111111111111
// 0000000000000000000000000000000000000000000000000000000000000000