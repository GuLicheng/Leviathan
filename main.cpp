#include <experimental/nom/combinator.hpp>
#include <experimental/nom/parser.hpp>
#include <leviathan/type_caster.hpp>
#include <print>
#include <functional>
#include <leviathan/meta/type.hpp>


int main(int argc, char const *argv[])
{
    auto p = std::make_pair(2, std::string("This sentence is long enough and the memory is allocated on heap."));

    auto [a, b] = std::move(p);

    std::print("a = {}, b = {}, p = {}\n", a, b, p);

    return 0;
}
