#include <lv_cpp/math/string_opt.hpp>
// #include <lv_cpp/io/console.hpp>

#include <iostream>

#define PrintString(x) (std::cout << x << std::endl)

namespace ls = leviathan::string;

void test1();
void test2();

int main()
{
    test2();

}

void test1()
{
    std::string str0 = "   hello world!";
    std::string str1 = " \t\n\t   ";
    std::string str2 = "hello world!   ";
    std::string str3 = "hello world!";
    PrintString(ls::trim(str0));
    PrintString(ls::trim(str1));
    PrintString(ls::trim(str2));
    PrintString(ls::trim(str3));

    PrintString(ls::trim(std::move(str0)));
    PrintString(ls::trim(std::move(str1)));
    PrintString(ls::trim(std::move(str2)));
    PrintString(ls::trim(std::move(str3)));


    std::string_view sv0 = "   hello world!";
    std::string_view sv1 = " \t\n\t   ";
    std::string_view sv2 = "hello world!   ";
    std::string_view sv3 = "hello world!";

    PrintString(ls::trim(sv0));
    PrintString(ls::trim(sv1));
    PrintString(ls::trim(sv2));
    PrintString(ls::trim(sv3));
}

void test2()
{
    std::string str = "\n hello world! \t";
    PrintString(str);
    auto address1 = (int*)&str[0];
    str = ls::trim(std::move(str));
    auto address2 = (int*)&str[0];
    PrintString(address1);
    PrintString(address2);
    PrintString(str);
}