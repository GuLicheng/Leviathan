#include <leviathan/print.hpp>
#include <leviathan/extc++/math.hpp>
#include <leviathan/extc++/string.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>
#include <algorithm>
#include <vector>

int main()
{

    std::string s = "Hello";
    auto s1 = std::views::repeat(s, 2) 
            | std::views::join 
            | std::ranges::to<std::string>();
    Console::WriteLine(leviathan::string::repeat("This is const char* ? no !", 2));
}


























