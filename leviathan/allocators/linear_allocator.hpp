#pragma once

#include <memory_resource>

namespace leviathan::alloc
{
    // The final keyword may optimize the virtual function. 
    struct linear_buffer final : public std::pmr::monotonic_buffer_resource
    {
    };

    template <typename T>
    class linear_allocator 
    {
        inline static auto buffer = new linear_buffer(1024 * 1024 * 2);

    public:

        using value_type = T;

        template <typename U>
        struct rebind { using other = linear_allocator<U>; };

        linear_allocator() = default;

    private:

        linear_buffer* m_buffer = buffer;

    };
}
