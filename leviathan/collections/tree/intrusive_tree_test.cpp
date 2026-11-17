#include <leviathan/collections/detail/avl_hook.hpp>
#include <leviathan/collections/tree/intrusive_tree.hpp>
#include <iostream>
#include <random>
#include <catch2/catch_all.hpp>

struct Student
{
    size_t rank;

    std::string name;

    [[=cpp::refl::tag("treehook")]]
    cpp::collections::intrusive::avl_hook treehook;

    Student(size_t r, const std::string& n) : rank(r), name(n) {}

    bool operator<(const Student& other) const
    {
        return rank < other.rank;
    }

    struct Drawer
    {
        static std::string operator()(const cpp::collections::intrusive::avl_hook* hookptr)
        {
            auto& s = *cpp::collections::from_hook<^^Student::treehook>(const_cast<cpp::collections::intrusive::avl_hook*>(hookptr));
            return std::format("{}:{}({})", s.rank, s.name, s.treehook.balance);
        }
    };
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

std::vector<Student> RandomStudents(size_t n)
{
    std::random_device rd;

    std::vector<Student> result;

    for (size_t i = 0; i < n; ++i)
    {
        size_t id = rd() % (n * 10);
        result.push_back({id, "Student" + std::to_string(id)});
    }

    return result;
}

TEST_CASE("avltree")
{
    using AVLTree = cpp::collections::intrusive::tree<cpp::refl::search_tag(^^Student, "treehook")>;

    auto avltree = AVLTree{};
    auto sources = RandomStudents(20);

    for (auto& student : sources)
    {
        avltree.insert(student);
    }

    // Get the root of the AVL tree from the anchor node using std::meta::info to avoid private member access checking
    constexpr auto info = cpp::refl::member_named(^^AVLTree, "m_anchor");
    auto root = avltree.[:info:].parent();

    CHECK(cpp::collections::intrusive::detail::avl_checker{}(root));

    auto ids = avltree | std::views::transform(&Student::rank) | std::ranges::to<std::vector>();
    CHECK(std::ranges::is_sorted(ids));

    std::cout << avltree.draw(Student::Drawer()) << std::endl;
}


