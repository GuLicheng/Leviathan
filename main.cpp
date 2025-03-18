#include <leviathan/extc++/all.hpp>
#include <leviathan/print.hpp>


void Show(auto rg)
{
    for (auto&& i : rg)
    {
        Console::WriteLine(i);
    }
}

void TestIndirect()
{
    std::vector<int*> ptrs = { new int(1), new int(2), new int(3) };
    auto rg = ptrs 
            | leviathan::views::compose(leviathan::ranges::indirection, leviathan::ranges::to_string)
            | std::views::join
            | std::ranges::to<std::string>();
    Console::WriteLine(rg);
}

void TestComposeAndProj()
{
    constexpr auto plus_one = [](int x) { return x + 1; };
    constexpr auto twice = [](int x) { return x * 2; };
    static_assert(leviathan::ranges::composition(twice, plus_one, plus_one)(3) == 10);
    static_assert(leviathan::ranges::projection(twice, plus_one, plus_one)(3) == 8);
}

void TestRemove()
{
    std::vector vec = { 1, 2, 3, 4, 5 };
    auto rg = vec | leviathan::views::remove(3);
    Show(rg);
}

void TestReplace()
{
    std::vector vec = { 1, 2, 3, 4, 5 };
    auto rg = vec | leviathan::views::replace(3, 10);
    Console::WriteLine(rg);
}

int main(int argc, char const *argv[])
{
    TestRemove();
    return 0;
}



