#pragma once

#include <iostream>
#include <functional>
#include <compare>

#define PrintLn(x) (std::cout << x << '\n')

template <bool Report = false, int CopyThrowExceptionCount = -1, int MoveThrowExceptionCount = -1>
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
        if constexpr (Report) std::cout << "default init: " << val << '\n';
    }
    
    explicit Int32(int x) : val{ x }
    {
        ++int_constructor; 
        if constexpr (Report) std::cout << "int init: " << val << '\n';
    }

    Int32(const Int32& rhs) : val{ rhs.val }
    { 
        ++copy_constructor; 
        if constexpr (Report) std::cout << "copy_constructor" << val << '\n';

        if constexpr (CopyThrowExceptionCount != -1)
        {
            struct CopyConstructorException { };
            if (copy_constructor >= CopyThrowExceptionCount)
            {
                throw CopyConstructorException();
            }
        }
    }

    Int32(Int32&& rhs) noexcept(MoveThrowExceptionCount == -1) : val{ std::exchange(rhs.val, 0) }
    { 
        ++move_constructor; 
        if constexpr (Report) std::cout << "move_constructor" << val << '\n';    

        if constexpr (MoveThrowExceptionCount != -1)
        {
            struct MoveConstructorException { };
            if (move_constructor >= MoveThrowExceptionCount)
            {
                throw MoveConstructorException();
            }
        }

    }

    Int32& operator=(const Int32& rhs) 
    {
        this->val = rhs.val; 
        ++copy_assignment;
        if constexpr (Report) std::cout << "copy_assignment :" << val << std::endl;
        return *this;
    }

    Int32& operator=(Int32&& rhs) noexcept
    { 
        this->val = std::exchange(rhs.val, 0);
        ++move_assignment;
        if constexpr (Report) std::cout << "move_assignment :" << val << std::endl;
        return *this;
    }

    operator int() const noexcept
    { return this->val; }

    Int32 operator+(Int32 rhs) const noexcept
    { return Int32{ this->val + rhs.val }; }

    ~Int32() 
    { 
        ++destructor; 
        if constexpr (Report) std::cout << "destructor" << val << '\n';   
    }

    friend std::ostream& operator<<(std::ostream& os, Int32 f)
    {
        return os << f.val;
    }

    constexpr auto operator<=>(const Int32&) const noexcept = default;

    constexpr std::size_t hash_code() const noexcept
    { return static_cast<std::size_t>(val); }

    struct HashType
    {
        constexpr std::size_t operator()(const Int32& i) const noexcept
        { return static_cast<std::size_t>(i.val); }
    };

};


namespace std 
{
    template <bool B>
    struct hash<::Int32<B>>
    {
        constexpr auto operator()(const ::Int32<B>& f) const noexcept
        { return f.hash_code(); }
    };
}


template <bool Report = false>
struct MoveCounter
{
    inline static int count = 0;
    MoveCounter() = default;
    MoveCounter(MoveCounter&&) 
    {
        ++count;
        if constexpr (Report) PrintLn(count);
    }

    MoveCounter(const MoveCounter&) = delete;
};

struct NoDefaultConstructable
{
    NoDefaultConstructable(int) { }
    constexpr bool operator==(const NoDefaultConstructable&) const noexcept
    { return true; }
};

namespace std 
{
    template <>
    struct hash<::NoDefaultConstructable>
    {
        constexpr auto operator()(const ::NoDefaultConstructable&) const noexcept
        { return 0; }
    };
}











