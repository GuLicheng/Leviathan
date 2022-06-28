#include <iostream>

#include "except_allocator.hpp"
#include <unordered_set>
#include <list>
#include <forward_list>
#include <set>

int main()
{
    std::list<int, RecordAllocator<int>> ls;
    std::forward_list<int, RecordAllocator<int>> fls;
    fls.push_front(1);
    Report();
}

