#include <leviathan/collections/detail/avl_hook.hpp>
#include <leviathan/collections/tree/intrusive_tree.hpp>
#include <iostream>
#include <catch2/catch_all.hpp>

struct Student
{
    int rank;

    std::string name;

    [[=cpp::refl::tag("treehook")]]
    cpp::collections::intrusive::avl_hook treehook;

    Student(int r, const std::string& n) : rank(r), name(n) {}

    bool operator<(const Student& other) const
    {
        return rank < other.rank;
    }
};

std::vector<Student> students =
{
    {
        {1, "Alice"},
        {2, "Bob"},
        {3, "Charlie"},
        {4, "David"},
        {5, "Eve"},
        {6, "Frank"},
        {7, "Grace"}
    }
};

TEST_CASE("avltree")
{
    using AVLTree = cpp::collections::intrusive::tree<cpp::refl::search_tag(^^Student, "treehook")>;

    auto avltree = AVLTree{};

    for (auto& student : students)
    {
        avltree.insert(student);
    }

    for (auto it = avltree.begin(); it != avltree.end(); ++it)
    {
        std::cout << it->rank << " " << it->name << "\n";
    }

    constexpr auto info = cpp::refl::member_named(^^AVLTree, "m_anchor");
    auto root = avltree.[:info:].parent();

    CHECK(cpp::collections::intrusive::detail::avl_checker{}(root));
    CHECK(avltree.size() == students.size());
}


