#include <leviathan/annotations/all.hpp>
#include <print>

int main(int argc, char const *argv[])
{
    std::println("{}", cpp::refl::pascal_case("hello_world"));
    std::println("{}", cpp::refl::pascal_case("_hello_world"));
    std::println("{}", cpp::refl::pascal_case("hello_world_"));
    std::println("{}", cpp::refl::pascal_case("hello__world"));

    std::println("{}", cpp::refl::kebab_case("hello_world"));
    std::println("{}", cpp::refl::kebab_case("_hello_world"));
    std::println("{}", cpp::refl::kebab_case("hello_world_"));
    std::println("{}", cpp::refl::kebab_case("hello__world"));
    

    return 0;
}
