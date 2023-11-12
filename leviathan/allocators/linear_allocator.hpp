#pragma once

#include <memory_resource>
#include <memory>

namespace leviathan::alloc
{
    // The final keyword may optimize the virtual function. 
    struct monotonic_buffer final : public std::pmr::monotonic_buffer_resource
    {
        using std::pmr::monotonic_buffer_resource::monotonic_buffer_resource;
        using std::pmr::monotonic_buffer_resource::operator=;
    };

    template <typename T, std::size_t MaxObjectCount = 1024, bool IsGlobal = true>
    class monotonic_allocator 
    {
        inline static auto buffer = new monotonic_buffer(MaxObjectCount * sizeof(T));

    public:

        monotonic_allocator(monotonic_buffer* buffer) : m_buffer(buffer)  { }

        monotonic_allocator() : monotonic_allocator(buffer) { }

        using value_type = T;
        using is_always_equal = std::conditional_t<IsGlobal, std::true_type, std::false_type>;

        template <typename U>
        struct rebind { using other = monotonic_allocator<U, MaxObjectCount, IsGlobal>; };

        [[nodiscard]] T* allocate(std::size_t n)
        { return (T*)m_buffer->allocate(n * sizeof(T), alignof(T)); }

        void deallocate(T* p, std::size_t n)
        { m_buffer->deallocate(p, n * sizeof(T), alignof(T)); }

        constexpr bool operator==(const monotonic_allocator& rhs) const 
        { 
            if constexpr (IsGlobal) 
            {
                return true;
            }
            else    
            {
                return m_buffer == rhs.m_buffer; 
            }
        }

    private:

        monotonic_buffer* m_buffer;
    };
}
