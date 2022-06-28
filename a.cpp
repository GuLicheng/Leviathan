#include <type_traits>

#include <memory>

int main()
{
    std::allocator<int> alloc;
    int val;
    std::uninitialized_construct_using_allocator(&val, alloc, 3);
    std::cout << val << '\n';
}