#include <leviathan/checked_allocator.hpp>

int main(int argc, char const *argv[])
{

    leviathan::alloc::checked_allocator<int> alloc(1);

    auto ptr = alloc.allocate(5);

    for (int i = 0; i < 5; ++i)
    {
        // ptr[i] = i;
        alloc.construct(ptr + i, i);
        std::cout << ptr[i] << ' ';
        alloc.destroy(ptr + i);
    }

    alloc.deallocate(ptr, 5);

    std::cout << '\n' << alloc << '\n';

    return 0;
}
