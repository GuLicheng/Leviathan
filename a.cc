#include "generator.hpp"
#include <lv_cpp/io/console.hpp>
#include <vector>
#include <functional>

int main()
{
    std::function<void()> f;
    console::write_line(f);
}