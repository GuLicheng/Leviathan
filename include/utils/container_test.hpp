#pragma once

#include "struct.hpp"
#include "fancy_ptr.hpp"
#include "record_allocator.hpp"

#include <assert.h>

template <template <typename...> typename Container>
struct TestHash
{
    static void TestMemorySafe()
    {
        using CopyThrowException = CopyThrowExceptionInt<false, 2>;
        using HashTable = Container<CopyThrowException, CopyThrowException::HashType> 
        
        {
            try
            {
                HashTable c;
                for (int i = 0; i < 10; ++i)
                    c.insert(i);
            }
            catch(...)
            {

            }
        }

        assert(CopyThrowException::total_construct() == CopyThrowException::total_destruct());
    }

};






