#include <ranges>

#include <iostream>


int main()
{
    for (auto value : std::ranges::istream_view<int>(std::cin))
        std::cout << value << '\n';
}




