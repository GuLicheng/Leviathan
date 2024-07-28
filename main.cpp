#include <leviathan/io/file.hpp>
#include <leviathan/print.hpp>

int main(int argc, char const *argv[])
{
    auto s = std::string("H");

    s.append(0, 'W');
    s.append(5, '7');

    Console::WriteLine(s);

    return 0;
}
