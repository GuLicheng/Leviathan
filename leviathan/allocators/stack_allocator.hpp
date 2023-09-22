#pragma once

#include <memory>
#include <type_traits>

namespace leviathan::alloc
{
    template <typename T, size_t N, bool ThrowException = true>
    class stack_allocator
    {
        struct throw_exception_t : std::bool_constant<ThrowException> { };

    private:

        constexpr T* allocate_impl(size_t n, std::true_type)
        {
            if (n != N || m_alloced)
            {
                throw std::bad_alloc();
            }
            m_alloced = true;
            return reinterpret_cast<T*>(&m_raw);
        }

        constexpr T* allocate_impl(size_t n, std::false_type)
        {
            if (n != N || m_alloced)
            {
                return nullptr;
            }
            m_alloced = true;
            return reinterpret_cast<T*>(&m_raw);
        }

    public:

        // All member for allocator_traits has default value except value_type.
        using value_type = T;

        template <typename U>
        struct rebind { using other = stack_allocator<U, N, ThrowException>; };

        constexpr stack_allocator() = default;

        constexpr stack_allocator(const stack_allocator&) = delete;

        constexpr T* allocate(size_t n)
        { return allocate_impl(n, throw_exception_t()); }

        constexpr void deallocate(void* p, size_t n)
        { }

        static consteval size_t max_size()  
        { return N; }

        constexpr friend bool operator==(const stack_allocator& lhs, const stack_allocator& rhs) noexcept
        { return &lhs.m_raw[0] == &rhs.m_raw[0]; }

    private:

        alignas(T) unsigned char m_raw[sizeof(T) * N];
        bool m_alloced;    // Allocate all memory once.

    };
} // namespace leviathan::alloc

