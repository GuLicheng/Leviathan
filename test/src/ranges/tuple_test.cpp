
#include <lv_cpp/io/console.hpp>
#include <lv_cpp/tuples/tuple_extend.hpp>
#include <lv_cpp/utils/test.hpp>

#include <iostream>
#include <tuple>
#include <functional>

using namespace leviathan::tuple;

constexpr int add1(int a, int b) { return a + b; }

int add2(int a, int b) { return a + b; }

int mul1(int a, int b) { return a * b; }

void test_for_tuple();

void test_for_pair();

void test_for_complier_time();

int main()
{
    test_for_pair();
    test_for_tuple();
    test_for_complier_time();
}



void test_for_complier_time()
{
    constexpr auto t1 = std::make_tuple(1, 2, 3);
    constexpr auto t2 = std::make_tuple(1, 2, 3);
    constexpr auto c = tuple_inner_preduct(t1, t2, std::plus<>(), std::plus<>(), 0);
    constexpr auto d = tuple_reduce(t1, std::plus<>(), 0);
    static_assert(c == 12);
    static_assert(d == 6);
}

void test_for_pair()
{
    auto p1 = std::make_pair(1, 2);
    auto p2 = std::make_pair(3, 4);
    console::write_lines_multi(p1, p2);
    auto res = tuple_inner_preduct(p1, p2, ::mul1, ::mul1, 1);
    ASSERT_EQ(24, res);
}


void test_for_tuple()
{
    auto f = ::add2;
    int x;
    std::cout << "Please input x for (1, 2, 3, 4, 5, 6, x): \n";
    std::cin >> x;
    auto t1 = std::make_tuple(1, 2, 3, 4, 5, 6, x);
    console::write_line_multi("Before reverse: ", t1);
    auto t2 = reverse_tuple_by_copy(t1);
    console::write_line_multi("After reverse: ", t2);
    std::cout << tuple_inner_preduct(t1, t2, ::add2, ::add2, 0) << std::endl;
}

