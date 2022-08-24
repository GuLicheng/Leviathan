#include <ranges>

#include <vector>


int main()
{
    std::vector values { 1, 2, 3, 4 };
    auto adaptor = std::views::take(3);
}




