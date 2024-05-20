#include <variant>
#include <iostream>

struct Leaf { };
struct Node;

using Tree = std::variant<Leaf, Node*>;

struct Node {
    int value;
    Tree left;
    Tree right;
};

template <typename... Fs>
struct Overload : Fs... {
    using Fs::operator()...;
};

int GetLeavesCount(const Tree& tree) {
    return std::visit(Overload(
        [](const Leaf&) static { return 1; },
        [](this const auto& self, const Node* n) -> int {
            return std::visit(self, n->left) + std::visit(self, n->right);
        }
    ), tree);
}


int main(int argc, char const *argv[])
{
    Tree t;

    std::cout << std::format("{}\n", GetLeavesCount(t));

    constexpr auto sz = sizeof(Tree);

    return 0;
}


