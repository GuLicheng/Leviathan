#pragma once

#include <memory_resource>
#include <memory>

namespace leviathan::alloc
{
    // The final keyword may optimize the virtual function. 
    struct monotonic_buffer final : public std::pmr::monotonic_buffer_resource
    {
    };

    template <typename T, std::size_t MaxObjectCount = 1024>
    class monotonic_allocator 
    {
        inline static auto buffer = new monotonic_buffer(MaxObjectCount * sizeof(T));

    public:

        using value_type = T;
        using is_always_equal = std::true_type;

        monotonic_allocator() = default;

        [[nodiscard]] T* allocate(std::size_t n)
        { return m_buffer->allocate(n * sizeof(T), alignof(T)); }

        void deallocate(T* p, std::size_t n)
        { m_buffer->deallocate(p, n * sizeof(T), alignof(T)); }

        consteval size_t max_size() const
        { return MaxObjectCount; }

    private:

        monotonic_buffer* m_buffer = buffer;

    };
}
