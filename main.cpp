#include <memory>
#include <iostream>

#include <leviathan/collections/static_vector.hpp>

int main(int argc, char const *argv[])
{
    leviathan::collections::static_vector<int, 32> s;

    for (int i = 0; i < s.capacity(); ++i)
        s.emplace_back(i);

    for (int i = 0; i < s.size(); ++i)
        std::cout << s[i] << '\n';

    std::cout << "OK\n";

    return 0;
}
