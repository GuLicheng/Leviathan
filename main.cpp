#include <leviathan/extc++/all.hpp>
#include <algorithm>
#include <leviathan/print.hpp>


int main(int argc, char const *argv[])
{
    std::ranges::copy(
        std::views::iota(0, 100) | 
        leviathan::views::format | 
        std::views::join_with(' '),
        leviathan::file_iterator("../a.txt", std::ios::out) 
    );


    return 0;
}
