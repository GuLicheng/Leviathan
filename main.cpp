
#include <optional>
#include <print>
#include <meta>
#include <iostream>
#include <span>
#include <set>

#include <leviathan/extc++/meta.hpp>
#include <leviathan/collections/core/node_interface.hpp>

// https://github.com/boostorg/intrusive/tree/develop/include/boost/intrusive

struct PyGC_Head
{
    PyGC_Head* gc_next;
    PyGC_Head* gc_prev;
};

constexpr auto info = ^^PyGC_Head::gc_next;

struct Student : cpp::collections::list_node_interface, IStudent
{
    int Id = -1;

    [[=cpp::refl::member_hook("list")]]
    std::array<Student*, 2> ListHook;  // As header

    [[=cpp::refl::member_hook("tree")]]
    std::array<Student*, 3> TreeHook;  // As header

    Student(int id) : Id(id)
    {

    }

    int getId() const override { return Id; }
};



int main()
{
    constexpr auto mem1 = cpp::refl::search_member_hook(^^Student, "list");
    constexpr auto mem2 = cpp::refl::search_member_hook(^^Student, "tree");
    std::cout << display_string_of(mem1) << '\n';
    std::cout << display_string_of(mem2) << '\n';

    Student s1(1), s2(2), s3(3);

    s2.insert_after(&s1);
    s3.insert_after(&s2);

    cpp::collections::list_node_iterator<Student> it(&s1);

    std::cout << it->getId() << '\n';
    ++it;
    std::cout << it->getId() << '\n';
    ++it;
    std::cout << it->getId() << '\n';

    std::println("Done");
}


