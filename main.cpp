#include <leviathan/print.hpp>
#include <leviathan/utils/controllable_value.hpp>
#include <leviathan/allocators/checked_allocator.hpp>
#include <vector>

using Alloc = leviathan::alloc::checked_allocator<int, leviathan::alloc::alloc_spec(7)>;

int main(int argc, char const *argv[])
{
    std::vector<int, Alloc> vec = { 1, 2, 3 };

    for (int i = 0; i < 100; ++i)
        vec.emplace(vec.begin(), 0);

    auto v = vec;

    ::println("{} - {}", vec, leviathan::alloc::check_memory_alloc());

    return 0;
}
// 1111111111111111111111111111111111111111111111111111111111111111
// 0000000000000000000000000000000000000000000000000000000000000000