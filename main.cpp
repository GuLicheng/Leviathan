#include <print>
#include <ranges>
#include <tuple>
#include <leviathan/extc++/meta.hpp>

struct Base1 {};
struct Base2 : Base1 {};
struct Derived : Base2, std::string {};


int main(int argc, char const *argv[])
{
    template for (constexpr auto base : define_static_array(cpp::refl::all_bases_of<Derived>()))
    {
        std::print("{}\n", display_string_of(base));
    }

    return 0;
}



















