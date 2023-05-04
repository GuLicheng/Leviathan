#include <iostream>
#include <string>
#include <concepts>
// #include "base.hpp"
#include <lv_cpp/callable_container.hpp>
#include <lv_cpp/struct.hpp>
#include <catch2/catch_all.hpp>

class Sum
{
public:
    int operator()(int a, int b) const { return a + b; }
    int X = 0;
};


void report(int a) { std::cout << (a); }

const Sum s;
Sum s2;
int a = 1;
callable_container callables;


TEST_CASE("lambda")
{
    // lambda 
    callables.register_handler("lambda", [](int a, int b) { return a + b; });
    auto p = callables.call_by_name<int>("lambda", a, 2);
    REQUIRE(*p == 3);
}

TEST_CASE("C-style function")
{
    callables.register_handler("c_function", report);
    callables.call_by_name<void>("c_function", 100);
}

TEST_CASE("member function")
{
    {
        callables.register_handler(".operator()", &Sum::operator());
        auto p = callables.call_by_name<int>(".operator()", &s, 1, 2);
        REQUIRE(*p == 3);
    }

    {   
        callables.register_handler("overload operator()", s2);
        auto p = callables.call_by_name<int>("overload operator()", 1, 2);
        REQUIRE(*p == 3);
    }
}

TEST_CASE("pass by value")
{
    auto consume_string = [](std::string s) { s.clear(); };
    std::string res = "hello world";
    callables.register_handler("consume_string", consume_string);
    callables.call_by_name<void>("consume_string", res);
    REQUIRE(res == "hello world");
}

TEST_CASE("pass by reference")
{
    auto consume_string = [](std::string& s) { s = "!"; };
    std::string res = "hello world";
    callables.register_handler("consume_string1", consume_string);
    callables.call_by_name<void>("consume_string1", std::ref(res));
    REQUIRE(res == "!");
}

TEST_CASE("capture")
{
    std::string value = "Hello";
    std::string ref = "Hello";
    auto capture_string = [value, &ref]() mutable { value.clear(), ref.clear(); };
    callables.register_handler("capture_string", capture_string);
    callables.call_by_name<void>("capture_string");
    callables.call_by_name<void>("capture_string");
    REQUIRE(value == "Hello");
    REQUIRE(ref == "");
}

TEST_CASE("object free")
{
    {
        Int32<> i{ 0 };
        callables.register_handler("capture_int", [i]() { });
        callables.call_by_name<void>("capture_int");
        callables.call_by_name<void>("capture_int");
        callables.call_by_name<void>("capture_int");
    }

    REQUIRE(Int32<>::total_construct() == Int32<>::total_destruct());

    {
        auto tuple = [](std::tuple<int> t) { return std::get<0>(t); };
        callables.register_handler("tuple", tuple);
        auto p = callables.call_by_name<int>("tuple", std::make_tuple(1));
        REQUIRE(*p == 1);
    }   

    {
        callables.register_handler("capture_Int32", [](int i) { return Int32<>(i); });
        auto p = callables.call_by_name<Int32<>>("capture_Int32", -1);
        std::cout << "Int32<> = " << p->val << '\n';
    }

    REQUIRE(Int32<>::total_construct() == Int32<>::total_destruct());
}

