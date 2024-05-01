#include <leviathan/algorithm/all.hpp>
#include <leviathan/print.hpp>

using Point = std::pair<int, int>;

namespace cxx { }

int main(int argc, char const *argv[])
{
    std::vector<Point> v;

    v.emplace_back(1, 1);

    leviathan::algorithm::ranges::linear_search(v, { 1, 2 });
    leviathan::algorithm::linear_search(v.begin(), v.end(), { 1, 2 });

    return 0;
}
// 1111111111111111111111111111111111111111111111111111111111111111
// 0000000000000000000000000000000000000000000000000000000000000000