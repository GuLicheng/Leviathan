#include <vector>
#include <numeric>
#include <ranges>
#include <iostream>

class Solution {
public:
    static int compress(const std::vector<char>& chars) {

        auto calculator = [](auto rg) {
            auto sz = std::ranges::size(rg);
            return sz == 1 ? 1 : 1 + std::to_string(sz).size();
        };

        auto rg = chars | std::views::chunk_by(std::equal_to<>()) | std::views::transform(calculator);

        return std::reduce(rg.begin(), rg.end(), 0);
    }
};

int main(int argc, char const *argv[])
{
    auto a = Solution::compress(std::vector<char>{'a', 'a', 'b', 'b', 'c', 'c', 'c'});
    std::cout << a << '\n';
    return 0;
}
