#include <leviathan/extc++/all.hpp>
#include <algorithm>
#include <leviathan/print.hpp>

int main(int argc, char const *argv[])
{
    std::ranges::copy(
        std::views::iota(0, 100) | 
        cpp::views::format | 
        std::views::join_with(' '),
        cpp::file_iterator("../a.txt", std::ios::out) 
    );

    std::identity i;

    return 0;
}
