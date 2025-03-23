#include <leviathan/collections/list/skip_list.hpp>
#include <leviathan/print.hpp>

using namespace leviathan::collections;

int main(int argc, char const *argv[])
{
    skip_set<int> sl;

    sl.emplace(1);
    sl.emplace(2);
    sl.emplace(8);
    sl.emplace(-1);

    Console::WriteLine(sl);

    return 0;
}
