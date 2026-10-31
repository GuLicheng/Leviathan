
#include <optional>
#include <print>
#include <meta>
#include <iostream>
#include <span>
#include <set>

#include <leviathan/extc++/meta.hpp>
#include <leviathan/collections/core/node_interface.hpp>

// https://github.com/boostorg/intrusive/tree/develop/include/boost/intrusive


struct list_node : cpp::collections::list_node_interface
{
    std::array<list_node*, 2> links;

    constexpr list_node() { reset(); }

    constexpr ~list_node() { }
};

struct Student : cpp::collections::list_node_interface
{
    std::array<Student*, 2> links;

    int Id = -1;

    list_node Hook;  // As header

    Student(int id) : Id(id)
    {
    }
};


int main()
{
    Student s1(1), s2(2), s3(3);

    s2.Hook.insert_after(&s1.Hook);
    s3.Hook.insert_after(&s2.Hook);

    using Iterator = cpp::collections::list_node_iterator<Student>;

    static_assert(std::bidirectional_iterator<Iterator>);

    auto it = Iterator(&s1);

    std::println("Current student ID: {}", it->Id);
    ++it;
    std::println("Current student ID: {}", it->Id);
    ++it;
    std::println("Current student ID: {}", it->Id);

    std::println("Test over");
}


