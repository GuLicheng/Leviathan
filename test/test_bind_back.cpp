#include <leviathan/utils/bind_back.hpp>
#include <assert.h>

void test_simple_function_with_two_arguments()
{
    auto minus = [](int a, int b) { return a - b; };
    auto minus_5 = leviathan::bind_back(minus, 5);
    assert(minus_5(10) == 5);
}

int main(int argc, char const *argv[])
{
    test_simple_function_with_two_arguments();
    return 0;
}


