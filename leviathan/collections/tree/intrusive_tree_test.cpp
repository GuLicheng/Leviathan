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
            return std::format("{}|h={}", s.rank, s.treehook.balance);
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
    auto sources = RandomStudents(2000);

    for (auto& student : sources)
    {
        avltree.insert(student);
    }

    CHECK(cpp::collections::intrusive::detail::avl_checker{}(avltree.root()));

    auto ids = avltree | std::views::transform(&Student::rank) | std::ranges::to<std::vector>();
    CHECK(std::ranges::is_sorted(ids));

    auto render = cpp::collections::intrusive::detail::tree_mermaid_render();
    std::cout << render(avltree.root(), Student::Drawer()) << std::endl;

    // erase
    avltree.clear();

    for (auto& student : students)
    {
        avltree.insert(student);
    }

    // auto render = cpp::collections::intrusive::tree_ascii_render();
    // std::cout << render.render_mermaid(avltree.root(), Student::Drawer()) << std::endl;
    
    for (auto it = avltree.begin(); it != avltree.end(); it = avltree.erase(it))
    {
        if (std::popcount(avltree.size()) == 1)
        {
            CHECK(cpp::collections::intrusive::detail::avl_checker{}(avltree.root()));
        }
    }

    CHECK(avltree.size() == 0);
}

