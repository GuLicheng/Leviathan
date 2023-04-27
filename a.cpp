#include <iostream>
#include <utility>
#include "include/lv_cpp/collections/internal/buffer.hpp"
// #include "thirdpart/catch.hpp"

#include "../include/lv_cpp/struct.hpp"

#define REQUIRE assert

using AllocatorT = std::allocator<int>;
AllocatorT allocator;

void func()
{
    leviathan::collections::buffer<int, AllocatorT> buffer;

    for (int i = 0; i < 10; ++i)
    {
        buffer.emplace_back(allocator, i);
    }
    
    int i = 0;

    for (auto value : buffer)
    {
        REQUIRE(value == i++);
    }
}

void fun2()
{
    using T = Int32<false, 2, -1, true>;
    {
        std::allocator<T> alloc;

        leviathan::collections::buffer<T, std::allocator<T>> buffer(alloc, 1);

        REQUIRE(std::is_nothrow_move_constructible_v<T> == false);
        REQUIRE(!T::Moveable);

        bool is_throw = false;

        try 
        {
            for (int i = 0; i < 10; ++i)
            {
                buffer.emplace_back(alloc, i);
            }
        }
        catch(...)
        {
            is_throw = true;
        }
        REQUIRE(is_throw == true);
        buffer.dispose(alloc);
    }

    REQUIRE(T::total_construct() == T::total_destruct());
}

int main(int argc, char const *argv[])
{
    func();
    std::cout << "Ok\n";
    return 0;
}
