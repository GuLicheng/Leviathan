#include <algorithm>
#include <string>
#include <iostream>
#include <regex>
std::string str1 = "hello";
std::string str2 = "hello";

int main()
{
    auto iter = std::mismatch(str1.begin(), str1.end(), str2.begin());
    std::cout << std::equal(str1.begin(), str1.end(), str2.begin());
}