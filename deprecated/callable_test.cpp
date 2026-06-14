#include <iostream>
#include <string>
#include <concepts>
#include <leviathan/callable.hpp>
#include <leviathan/utils/controllable_value.hpp>
#include <catch2/catch_all.hpp>

class Sum
{
public:
    int operator()(int a, int b) const { return a + b; }
    int X = 0;
};


void report(int* a) { (*a)++; }

const Sum s;
Sum s2;
int a = 1;
cpp::callable callables;


TEST_CASE("lambda")
{
    // lambda 
    callables.register_handler("lambda", [](int a, int b) { return a + b; });
    auto p = callables.call<int>("lambda", a, 2);
    REQUIRE(p == 3);
}

TEST_CASE("static lambda")
{
    // static lambda
    callables.register_handler("static_lambda", [](int a, int b) static { return a + b; });
    auto p = callables.call<int>("static_lambda", 1, 2);
    REQUIRE(p == 3);
}

TEST_CASE("C-style function")
{
    int x = 100;
    callables.register_handler("c_function", report);
    callables.call<void>("c_function", &x);
    REQUIRE(x == 101);
}

TEST_CASE("member function")
{
    {
        callables.register_handler(".operator()", &Sum::operator());
        auto p = callables.call<int>(".operator()", &s, 1, 2);
        REQUIRE(p == 3);
    }

    {   
        callables.register_handler("overload operator()", s2);
        auto p = callables.call<int>("overload operator()", 1, 2);
        REQUIRE(p == 3);
    }
}

TEST_CASE("pass by value")
{
    auto consume_string = [](std::string s) { s.clear(); };
    std::string res = "hello world";
    callables.register_handler("consume_string", consume_string);
    callables.call<void>("consume_string", res);
    REQUIRE(res == "hello world");
}

TEST_CASE("pass by reference")
{
    auto consume_string = [](std::string& s) { s = "!"; };
    std::string res = "hello world";
    callables.register_handler("consume_string1", consume_string);
    callables.call<void>("consume_string1", std::ref(res));
    REQUIRE(res == "!");
}

TEST_CASE("capture")
{
    std::string value = "Hello";
    std::string ref = "Hello";
    auto capture_string = [value, &ref]() mutable { value.clear(), ref.clear(); };
    callables.register_handler("capture_string", capture_string);
    callables.call<void>("capture_string");
    callables.call<void>("capture_string");
    REQUIRE(value == "Hello");
    REQUIRE(ref == "");
}

TEST_CASE("object free")
{
    using Int32 = cpp::controllable_value<int>;

    {
        Int32 i{ 0 };
        callables.register_handler("capture_int", [i]() { });
        callables.call<void>("capture_int");
        callables.call<void>("capture_int");
        callables.call<void>("capture_int");
        callables.clear();
    }

    REQUIRE(Int32::total_construct() == Int32::total_destruct());

    {
        auto tuple = [](std::tuple<int> t) { return std::get<0>(t); };
        callables.register_handler("tuple", tuple);
        auto p = callables.call<int>("tuple", std::make_tuple(1));
        REQUIRE(p == 1);
        callables.clear();
    }   

    {
        callables.register_handler("capture_Int32", [](int i) { return Int32(i); });
        auto p = callables.call<Int32>("capture_Int32", -1);
        REQUIRE(p.m_value == -1);
        callables.clear();
    }

    REQUIRE(Int32::total_construct() == Int32::total_destruct());
}

