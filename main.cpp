#include <meta>
#include <initializer_list> 
#include <span>
#include <print>
#include <leviathan/annotations/all.hpp>
#include <leviathan/extc++/meta.hpp>
#include <list>

struct F
{
    [[=cpp::refl::default_array({1, 2, 3})]]
    std::list<int> ints;
};

int main(int argc, char const *argv[])
{
    auto f = cpp::refl::construct_struct<F>([](auto&&...) { });
    std::println("ints: {}", f.ints);
    return 0;
}
