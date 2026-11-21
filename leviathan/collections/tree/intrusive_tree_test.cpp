#include <leviathan/collections/hook/avl_hook.hpp>
#include <leviathan/collections/hook/red_black_hook.hpp>
#include <leviathan/collections/hook/treap_hook.hpp>
#include <leviathan/collections/tree/intrusive_tree.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <print>
#include <catch2/catch_all.hpp>

using cpp::collections::intrusive::avl_hook;
using cpp::collections::intrusive::red_black_hook;
using cpp::collections::intrusive::treap_hook;
using cpp::collections::intrusive::tree;
using cpp::collections::from_hook;
using cpp::collections::intrusive::detail::avl_mermaid_render;
using cpp::collections::intrusive::detail::red_black_mermaid_render;
using cpp::collections::intrusive::detail::treap_mermaid_render;
using cpp::collections::intrusive::detail::avl_checker;
using cpp::collections::intrusive::detail::red_black_checker;
using cpp::collections::intrusive::detail::treap_checker;


struct Student
{
    size_t rank;

    std::string name;

    [[=cpp::refl::tag("avlhook")]]
    avl_hook avlhook;
    
    [[=cpp::refl::tag("redblackhook")]]
    red_black_hook redblackhook;

    [[=cpp::refl::tag("treaphook")]]
    treap_hook treaphook;
    
    Student(size_t r, const std::string& n) : rank(r), name(n)
    {
        // avlhook.reset();
    }

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

        static std::string operator()(const treap_hook* hookptr)
        {
            auto& s = *from_hook<^^Student::treaphook>(const_cast<treap_hook*>(hookptr));
            return std::format("{}|p={}", s.rank, s.treaphook.priority);
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
        // size_t id = i;
        result.push_back({id, "Student" + std::to_string(id)});
    }
    std::sort(result.begin(), result.end());
    return result;
}

class Visualizer
{
    std::ofstream visual;
    
public:

    Visualizer(const std::string& filename) : visual(filename, std::ios::out | std::ios::binary) {}

    template <typename Render, typename Node>
    void draw(const Render& render, const Node* node)
    {
        visual << render(node, Student::Drawer()) << std::endl;
        visual << "\n--------------------\n" << std::endl;
    }

    void line(std::string msg)
    {
        visual << msg << std::endl;
        visual << "\n--------------------\n" << std::endl;
    }

};

void RemoveNodeFromTree(auto& tree, auto checker)
{
    for (auto it = tree.begin(); it != tree.end(); it = tree.erase(it))
    {
        if (std::popcount(tree.size()) == 1)
        {
            CHECK(checker(tree.root()));
        }
    }
    CHECK(tree.size() == 0);
}

void CheckOrder(auto& tree)
{
    auto ids = tree | std::views::transform(&Student::rank) | std::ranges::to<std::vector>();
    CHECK(std::ranges::is_sorted(ids));
}

TEST_CASE("avltree")
{
    using AVLTree = tree<cpp::refl::search_tag(^^Student, "avlhook")>;
    using RBTree = tree<cpp::refl::search_tag(^^Student, "redblackhook")>;
    using Treap = tree<cpp::refl::search_tag(^^Student, "treaphook")>;

    auto avltree = AVLTree{};
    auto rbtree = RBTree{};
    auto treap = Treap{};

    auto sources = RandomStudents(20000);  

    // for (auto& student : students)
    for (auto& student : sources)
    {
        avltree.insert(student);
        rbtree.insert(student);
        treap.insert(student);
    }

    CHECK(avl_checker{}(avltree.root()));
    CHECK(red_black_checker{}(rbtree.root()));

    CheckOrder(avltree);
    CheckOrder(rbtree);
    CheckOrder(treap);

    auto visualizer = Visualizer("E:\\Library\\Leviathan\\visual.md");

    // visualizer.draw(avl_mermaid_render(), avltree.root());
    // visualizer.draw(red_black_mermaid_render(), rbtree.root());
    // visualizer.draw(treap_mermaid_render(), treap.root());

    // erase
    avltree.clear();
    rbtree.clear();
    treap.clear();

    CHECK(avltree.size() == 0);
    CHECK(rbtree.size() == 0);
    CHECK(treap.size() == 0);

    for (auto& student : sources)
    // for (auto& student : students)
    { 
        // visualizer.line("Inserting student with rank: " + std::to_string(student.rank));
        // avltree.insert(student);
        rbtree.insert(student);
        // visualizer.draw(red_black_mermaid_render(), rbtree.root());
        treap.insert(student);
    }

    // RemoveNodeFromTree(avltree, avl_checker{});
    RemoveNodeFromTree(rbtree, red_black_checker{});
    RemoveNodeFromTree(treap, treap_checker{});

    // visualizer.draw(avl_mermaid_render(), avltree.root());
    visualizer.draw(red_black_mermaid_render(), rbtree.root());
    visualizer.draw(treap_mermaid_render(), treap.root());
}

