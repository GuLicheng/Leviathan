#include <lv_cpp/tuple_extend.hpp>

#include <iostream>
#include <tuple>
#include <functional>

using namespace leviathan;

constexpr int add1(int a, int b) { return a + b; }

int add2(int a, int b) { return a + b; }

int mul1(int a, int b) { return a * b; }

void test_for_tuple();

void test_for_pair();

int main()
{
    test_for_pair();
    test_for_tuple();
}



void test_for_pair()
{
    auto p1 = std::make_pair(1, 2);
    auto p2 = std::make_pair(3, 4);
    ::leviathan::print_tuple(std::cout, p1);
    ::leviathan::print_tuple(std::cout, p2);
    // ::leviathan::tuple_inner_preduct(p1, p2, ::add1, ::add1, 0);
    // std::cout << "-------------------------------\n";
    std::cout << '(' << 
        ::leviathan::tuple_inner_preduct(p1, p2, ::mul1, ::mul1, 1) 
        << ')' << std::endl;   
}


void test_for_tuple()
{
    auto f = ::add2;
    int x;
    std::cout << "Please input x for (1, 2, 3, 4, 5, 6, x): \n";
    std::cin >> x;
    auto t1 = std::make_tuple(1, 2, 3, 4, 5, 6, x);
    leviathan::print_tuple(std::cout, t1);
    std::cout << std::endl;
    auto t2 = reverse_tuple_by_copy(t1);
    leviathan::print_tuple(std::cout, t2);
    std::cout << std::endl;
    std::cout << leviathan::tuple_inner_preduct(t1, t2, ::add2, ::add2, 0) << std::endl;
}