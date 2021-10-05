#include <iostream>
int main()
{
    std::cout << "Hello";
    std::cout << "\033[?25h";
    std::cout << "\033[?25l";
    std::cout << "World";
}
