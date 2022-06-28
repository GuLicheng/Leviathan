#include <type_traits>
#include <iostream>
#include <memory>
#include "test/except_allocator.hpp"

int main()
{
    RecordPolymorphicAllocator<int> alloc;
    double val;
    double* p;
    p = std::uninitialized_construct_using_allocator(&val, alloc, 3.5);
    val = std::make_obj_using_allocator<double>(alloc, 3.14);
    std::cout << val << '\n';
    std::cout << p << '\n';
    Report();
}