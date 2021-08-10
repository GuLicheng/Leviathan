#pragma once

#include <iostream>
#include <functional>
#include <compare>

struct foo
{
private:
    inline static int default_constructor = 0;
    inline static int copy_constructor = 0;
    inline static int copy_assignment = 0;
    inline static int move_constructor = 0;
    inline static int move_assignment = 0;
    inline static int destructor = 0;
    inline static int int_constructor = 0;
public:

    int val;

    foo() : val{ 0 } 
    { std::cout << "default_constructor :" << ++default_constructor << std::endl;}
    
    foo(int x) : val{ x }
    { std::cout << "int_constructor : " << ++int_constructor << std::endl; }

    foo(const foo& rhs) : val{rhs.val}
    { std::cout << "copy_constructor :" << ++copy_constructor << std::endl;}

    foo(foo&& rhs) noexcept : val{ std::exchange(rhs.val, 0) }
    { std::cout << "move_constructor :" << ++move_constructor << std::endl;}

    foo& operator=(const foo& rhs) 
    {
        this->val = rhs.val; 
        std::cout << "copy_assignment :" << ++copy_assignment << std::endl;
        return *this;
    }

    foo& operator=(foo&& rhs) noexcept
    { 
        this->val = std::exchange(rhs.val, 0);
        std::cout << "move_assignment :" << ++move_assignment << std::endl;
        return *this;
    }

    ~foo() 
    { std::cout << "value is: " << val << " and destructor :" << ++destructor << std::endl;}

    friend std::ostream& operator<<(std::ostream& os, const foo& f)
    {
        return os << f.val;
    }

    auto operator<=>(const foo&) const noexcept = default;

};

namespace std 
{
    template <>
    struct hash<foo>
    {
        auto operator()(const foo& f) const noexcept
        { return std::hash<int>()(f.val); }
    };
}

struct empty_class { };


template <typename... Args>
class Base
{
};

template <typename T>
class Derived1 : public Base<T>
{
};

template <typename T>
struct Derived2 : public Base<Derived2<T>>
{
};

struct Derived3 final : Base<int>, Base<double>
{
};