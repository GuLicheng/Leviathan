#pragma once

#include <iostream>
#include <functional>
#include <compare>


#define PrintLn(x) (std::cout << x << '\n')

struct Int32
{
public:
    inline static int default_constructor = 0;
    inline static int copy_constructor = 0;
    inline static int copy_assignment = 0;
    inline static int move_constructor = 0;
    inline static int move_assignment = 0;
    inline static int destructor = 0;
    inline static int int_constructor = 0;
public:

    int static total_construct() 
    {
        return default_constructor + copy_constructor + move_constructor + int_constructor;
    }

    int static total_destruct()
    { 
        return destructor; 
    }

    int val;

    Int32() : val{ 0 } 
    { 
        ++default_constructor; 
    }
    
    explicit Int32(int x) : val{ x }
    {
        ++int_constructor; 
        PrintLn(int_constructor); 
    }

    Int32(const Int32& rhs) : val{ rhs.val }
    { ++copy_constructor; }

    Int32(Int32&& rhs) noexcept : val{ std::exchange(rhs.val, 0) }
    { ++move_constructor; }

    Int32& operator=(const Int32& rhs) 
    {
        this->val = rhs.val; 
        // std::cout << "copy_assignment :" << ++copy_assignment << std::endl;
        return *this;
    }

    Int32& operator=(Int32&& rhs) noexcept
    { 
        this->val = std::exchange(rhs.val, 0);
        // std::cout << "move_assignment :" << ++move_assignment << std::endl;
        return *this;
    }

    operator int() const noexcept
    { return this->val; }

    Int32 operator+(Int32 rhs) const noexcept
    { return Int32{ this->val + rhs.val }; }

    ~Int32() 
    { ++destructor; }

    friend std::ostream& operator<<(std::ostream& os, const Int32& f)
    {
        return os << f.val;
    }

    auto operator<=>(const Int32&) const noexcept = default;

    // [[nodiscard]] void* operator new(std::size_t count)
    // { return ::malloc(sizeof(Int32) * count); }

    // [[nodiscard]] void* operator new[](std::size_t count)
    // { return ::malloc(sizeof(Int32) * count); }
    
    // void operator delete( void* ptr ) noexcept
    // { ::operator delete(ptr); }

    // void operator delete[]( void* ptr ) noexcept
    // { ::operator delete[](ptr); }
};

namespace std 
{
    template <>
    struct hash<Int32>
    {
        auto operator()(const Int32& f) const noexcept
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