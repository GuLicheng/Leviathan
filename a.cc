#include <lv_cpp/io/console.hpp>
#include <vector>
#include <set>

std::vector vec{1, 2, 3, 4, 50};
std::set set{0, 0, 0, 1, 1};

int main()
{
    console::write_line(vec);
    console::write_line(set);
}