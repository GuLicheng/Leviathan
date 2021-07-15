#pragma once


#include <assert.h>
#include <string.h>

#include <iostream>
#include <memory>
#include <cstdlib>
#include <memory_resource>
#include <cstddef>
#include <vector>

namespace leviathan
{
    class bucket
    {
    public:
        const std::size_t BlockSize;
        const std::size_t BlockCount;

        bucket(std::size_t block_size, std::size_t block_count)
            : 
            BlockSize{ block_size }, 
            BlockCount{ block_count }, 
            m_ledger(block_size * block_count, false)
        {
            // allocate memory for this bucket
            const auto data_size = BlockSize * BlockCount;
            this->m_data = static_cast<std::byte*>(::operator new(data_size, std::nothrow));
            assert(this->m_data != nullptr);
            ::memset(this->m_data, 0, data_size);
        }

        ~bucket()
        {
            ::operator delete(this->m_data);
        }

        // Test if the pointer belong to this bucket
        bool belong(void* ptr) const noexcept
        {
            auto p = static_cast<std::byte*>(ptr);
            return p >= this->m_data && p <= this->m_data + BlockSize * BlockCount;
        }

        // Return nullptr if failed
        [[nodiscard]] void* allocate(std::size_t bytes) noexcept
        {
            // calculate the required number of blocks
            const auto n = 1 + ((bytes - 1) / BlockSize);
            const auto index = find_configuration_blocks(n);
            if (index == BlockCount) {
                return nullptr;
            }
            set_blocks_in_use(index, n);
            return this->m_data + (index * BlockSize);
        }

        void deallocate(void* ptr, std::size_t bytes) noexcept
        {
            const auto p = static_cast<const std::byte*>(ptr);
            const std::size_t dist = static_cast<std::size_t>(p - this->m_data);
            // Calculate block index from pointer distance
            const auto index = dist / BlockSize;
            // Calculate the required number of blocks
            const auto n = 1 + ((bytes - 1) / BlockSize);
            // Update the ledger
            set_block_free(index, n);
        }

    private:

        // Finds n free contiguous blocks in the ledger and returns the first block's index or BlockCount on failure
        std::size_t find_configuration_blocks(std::size_t n) const noexcept
        {
            assert(n == 1);

            for (std::size_t i = 0; i < this->m_ledger.size(); ++i)
            {
                if (this->m_ledger[i] == 0)
                    return i;
            }
            return BlockCount;
        }

        // Marks n blocks in the ledger as "in-use" starting at `index`
        void set_blocks_in_use(std::size_t index, std::size_t n) noexcept
        {
            assert(n == 1 && index < this->m_ledger.size());
            // for generic, index + n - 1 < this->m_ledger.size()
            this->m_ledger[index] = 1;
        }

        // Marks n blocks in the ledger as "free" starting at `index`
        void set_block_free(std::size_t index, std::size_t n) noexcept
        {
            assert(n == 1 && index < this->m_ledger.size());
            this->m_ledger[index] = 0;
        }

        // Actual memory for allocation
        std::byte* m_data{ nullptr };

        // Reserves one bit per block to indicate whether it is in-used
        // 0 is unused and 1 is used
        // std::byte* m_ledger{ nullptr };
        std::vector<bool> m_ledger;

    };



    template<typename _Ty>
    class Allocator {
    public:
        // inner type of data

        using value_type = std::remove_const_t<_Ty>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        //using reference = value_type&; // removed in c++20
        //using const_reference = const reference; // removed in c++20
        //using pointer = value_type*; // removed in c++20
        //using const_pointer = const pointer;  // removed in c++20

        // propagate_on_container_move_assignment(C++14)
        // is_always_equal(C++17)(deprecated in C++20)

        //template<typename _U>
        //struct rebind {
        //    typedef Allocator<_U> other; // type_cast if the type is difference(type not unique)
        //};  // removed in c++20

        Allocator() : m_bkt{ sizeof(_Ty), 8 }
        {
        }

        template<typename _otherAll>
        Allocator(const Allocator<_otherAll>&) noexcept 
            : m_bkt{ sizeof(_Ty), 8 }
        {
           // for Node-based container, List<int, Allocate<int>> -> rebind<ListNode<int>>
        };

        ~Allocator() = default;

        //apply memory 
        value_type* allocate(size_type num) {
            return (value_type*)m_bkt.allocate(num);
        }

        // relese memory
        void deallocate(value_type* p, size_type size) {
            m_bkt.deallocate(p, size);
        }

        bool operator==(const Allocator& rhs) const noexcept
        {
            return this == std::addressof(rhs);
        }

        bool operator!=(const Allocator& rhs) const noexcept
        {
            return !this->operator==(rhs);
        }

    private:
        bucket m_bkt;
    };

    template <typename T, std::size_t N>
    class PmrAllocator : public std::pmr::memory_resource
    {
    public:
        PmrAllocator()
            : m_bkt{ sizeof(T), N }
        {
        }

        virtual void*
        do_allocate(size_t bytes, size_t alignment)
        {
            return static_cast<void*>(this->m_bkt.allocate(bytes));
        }

        virtual void
        do_deallocate(void* p, size_t bytes, size_t alignment)
        {
            this->m_bkt.deallocate(p, bytes);
        }

        virtual bool
        do_is_equal(const memory_resource& other) const noexcept
        {
            return this == std::addressof(other);
        }

    private:
        bucket m_bkt;
    };

}