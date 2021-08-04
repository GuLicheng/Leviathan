#include <memory>
#include <iostream>

template <typename T>
struct MyAllocator
{
    using value_type = T;
    static MyAllocator select_on_container_copy_construction(const MyAllocator& rhs)
    {
        std::cout << "called\n";
        return rhs;
    }
};


int main(int argc, char const *argv[])
{
    MyAllocator<int> a, b;
    b = std::allocator_traits<MyAllocator<int>>::select_on_container_copy_construction(a);
    return 0;
}
