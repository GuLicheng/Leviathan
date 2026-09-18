#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <print>
#include <meta>

struct Color
{
    int r;
    int g;
    int b;

    Color(int r, int g, int b) : r(r), g(g), b(b) {}
    Color() : r(0), g(0), b(0) {}
    Color(const Color&) { std::println("Color copy constructor called"); }
    Color(Color&&) { std::println("Color move constructor called"); }
};

int main()
{
    std::optional<std::tuple<int, double>> opt = std::make_tuple(42, 3.14);

}
