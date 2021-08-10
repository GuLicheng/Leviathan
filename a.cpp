#include <memory>
#include <memory_resource>



int main()
{
    using T = std::allocator_traits<std::allocator<int>>;
    // std::pmr::polymorphic_allocator<int> alloc;
    // alloc.select_on_container_copy_construction
    // T::allocate
    // T::deallocate()
    // T::construct()
}