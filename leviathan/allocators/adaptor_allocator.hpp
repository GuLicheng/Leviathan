#pragma once

#include <memory_resource>

namespace leviathan::alloc
{
    template <typename MemoryResource>
    class adaptor_allocator
    {
        MemoryResource* m_resource;

        // std::pmr::polymorphic_allocator<

    public:

        adaptor_allocator(MemoryResource* resource) : m_resource(resource) { }



    };
} // namespace leviathan::alloc

