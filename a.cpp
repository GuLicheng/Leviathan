
#include <lv_cpp/utils/bigint.hpp>
#include <iostream>

using leviathan::bigint;

void test_for_copy_move()
{
    bigint b1{"123"};
    bigint b2{"456"};

    // & + &
    std::cout << (b1 + b2).string() << '\n';
    std::cout << "& + &\n";
    // && + &
    std::cout << "&& + &\n";
    std::cout << (bigint{"123"} + b1).string() << '\n'; 

    // & + &&
    std::cout << "& + &&\n";
    std::cout << (b1 + bigint{"123"}).string() << '\n'; 

    // && + &&
    std::cout << "&& + &&\n";
    std::cout << (bigint{"-123"} + bigint{"-123"}).string() << '\n'; 
}


int main()
{
    test_for_copy_move();
}