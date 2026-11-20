#include <leviathan/collections/detail/avl_hook.hpp>
#include <leviathan/collections/detail/red_black_hook.hpp>
#include <leviathan/collections/tree/intrusive_tree.hpp>
#include <iostream>
#include <random>
#include <catch2/catch_all.hpp>

using cpp::collections::intrusive::avl_hook;
using cpp::collections::intrusive::red_black_hook;
using cpp::collections::intrusive::tree;
using cpp::collections::from_hook;

struct Student
{
    size_t rank;

    std::string name;

    [[=cpp::refl::tag("avlhook")]]
    avl_hook avlhook;
    
    [[=cpp::refl::tag("redblackhook")]]
    red_black_hook redblackhook;

    Student(size_t r, const std::string& n) : rank(r), name(n) {}

    bool operator<(const Student& other) const
    {
        return rank < other.rank;
    }

    struct Drawer
    {
        static std::string operator()(const avl_hook* hookptr)
        {
            auto& s = *from_hook<^^Student::avlhook>(const_cast<avl_hook*>(hookptr));
            return std::format("{}|h={}", s.rank, s.avlhook.balance);
        }

        static std::string operator()(const red_black_hook* hookptr)
        {
            auto& s = *from_hook<^^Student::redblackhook>(const_cast<red_black_hook*>(hookptr));
            return std::format("{}", s.rank);
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
    using AVLTree = tree<cpp::refl::search_tag(^^Student, "avlhook")>;
    using RBTree = tree<cpp::refl::search_tag(^^Student, "redblackhook")>;

    auto avltree = AVLTree{};
    auto rbtree = RBTree{};

    auto sources = RandomStudents(20);

    for (auto& student : sources)
    {
        avltree.insert(student);
        rbtree.insert(student);
    }

    CHECK(cpp::collections::intrusive::detail::avl_checker{}(avltree.root()));

    auto ids = avltree | std::views::transform(&Student::rank) | std::ranges::to<std::vector>();
    CHECK(std::ranges::is_sorted(ids));

    auto render = cpp::collections::intrusive::detail::avl_mermaid_render();
    std::cout << render(avltree.root(), Student::Drawer()) << std::endl;

    // Split line
    std::cout << "--------------------" << std::endl;

    auto rbrender = cpp::collections::intrusive::detail::red_black_mermaid_render();
    std::cout << rbrender(rbtree.root(), Student::Drawer()) << std::endl;

    // erase
    avltree.clear();

    for (auto& student : students)
    {
        avltree.insert(student);
    }
    
    for (auto it = avltree.begin(); it != avltree.end(); it = avltree.erase(it))
    {
        if (std::popcount(avltree.size()) == 1)
        {
            CHECK(cpp::collections::intrusive::detail::avl_checker{}(avltree.root()));
        }
    }

    CHECK(avltree.size() == 0);
}

