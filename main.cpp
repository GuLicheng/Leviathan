#include "avl.hpp"
#include <set>
#include <iostream>
#include <memory_resource>
#include <format>
#include <memory>

template class avl_set<int>;

void REQUIRE(...) { }

int main()
{
    avl_set<int> t;

    std::set<int>::node_type s1;
    std::set<int> s2(s1);

    auto ret = s2.insert(std::move(s1));

    using T1 = decltype(ret);

    ret.node;

    s1 <=> s2;

    std::unique_ptr<std::pmr::memory_resource> p1(new std::pmr::monotonic_buffer_resource(1024));
    std::unique_ptr<std::pmr::memory_resource> p2(new std::pmr::monotonic_buffer_resource(2048));

    std::pmr::polymorphic_allocator<int> alloc1(p1.get());
    std::pmr::polymorphic_allocator<int> alloc2(p2.get());


    std::cout << "Ok\n";

}
