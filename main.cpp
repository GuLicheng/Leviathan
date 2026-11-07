
#include <optional>
#include <print>
#include <meta>
#include <iostream>
#include <span>
#include <set>

#include <leviathan/extc++/meta.hpp>
#include <leviathan/collections/core/node_interface.hpp>

// https://github.com/boostorg/intrusive/tree/develop/include/boost/intrusive

struct Student : cpp::collections::list_node_interface
{
    int Id = -1;

    [[=cpp::refl::member_hook("list")]]
    std::array<Student*, 2> ListHook;  // As header

    [[=cpp::refl::member_hook("tree")]]
    std::array<Student*, 3> TreeHook;  // As header

    Student(int id) : Id(id)
    {

    }
};



int main()
{
    constexpr auto mem1 = search_member_hook(^^Student, "list");
    constexpr auto mem2 = search_member_hook(^^Student, "tree");
    std::cout << display_string_of(mem1) << '\n';
    std::cout << display_string_of(mem2) << '\n';
}


