#include <lv_cpp/linq/linq.hpp>
// #include <lv_cpp/io/console.hpp>

#include <vector>
#include <list>
#include <string>

using namespace leviathan::linq;

struct integer
{
    int i;
    integer(int i) : i{i} 
    {
    }
    operator int() const
    { return i; }
    std::string name() const
    {
        return std::to_string(this->i) + "0";
    }
};

int main()
{
    std::vector<integer> arr;
    for (int i = 0; i < 10; ++i)
        arr.emplace_back(i);
    std::list ls(arr.begin(), arr.end());
    auto res = from(arr)
        .reverse()
        .where([](int x) { return x < 4 || x > 6; })
        .select([](integer x) { return x.name(); })
        .skip(1)
        .skip_while([](const std::string& str) { return str[0] > '5'; })
        .take(2)
        .for_each([](auto x) { std::cout << x << ' '; })
        ;

}