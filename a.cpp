#include <iostream>
#include <utility>
#include <lv_cpp/algorithm/combination.hpp>

void print_array(auto& x)
{
    for (auto val : x)
    {
        std::cout << val << ' ';
    }
    std::cout << '\n';
}

int main(int argc, char const *argv[])
{

    int arrays[] = { 0, 3, 5, 7, 9 };
    int n = 3;

    do {
        print_array(arrays);
    } while (leviathan::next_combination(arrays, arrays + n, arrays + 5));
    
    return 0;
}
