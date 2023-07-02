#include <ranges>
#include <iostream>
#include <leviathan/meta/template_info.hpp>

struct S
{
    struct 
    {
        int a;
        double b;
    };

    enum struct Mode
    {
        Int, 
        Double,
    } mode;
};

int main(int argc, char const *argv[])
{
    S s;

    s.mode = S::Mode::Double;

    using T = decltype(s.mode);

    PrintTypeInfo(T);

    return 0;
}
