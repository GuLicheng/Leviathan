#include "base.hpp"
#include <lv_cpp/concepts_extend.hpp>

int CountFileLines(const char* filename)
{
    std::fstream fs{ filename };
    return std::ranges::count(
        MakeRange(std::istreambuf_iterator<char>{fs}),
        '\n'
    );
}

int main(int argc, char const *argv[])
{
    Println(CountFileLines("a.py"));  // 21
    return 0;
}




