#include <ranges>
#include <string>
#include <functional>
#include <algorithm>
#include <iostream> 

class Solution {
public:

    int maximumLength(const std::string& s) {

        auto it = std::ranges::partition_point(
            std::views::iota(0, std::ssize(s)), 
            std::bind_front(&Solution::check, std::cref(s)));
        return *it < 2 ? -1 : *it - 1;    
    }

    static bool check(const std::string& s, int k) {
        int hash[128] = {};

        for (auto ck : s | std::views::chunk_by(std::ranges::equal_to())) {
            int length = std::ranges::size(ck);
            char ch = *ck.begin();
            hash[ch] += std::max(0, length - k + 1);
            if (hash[ch] >= 3) return true;
        }

        return false;
    }

};

int main(int argc, char const *argv[])
{
    
    std::pair<std::string, int> cases[] = {
        { "aaaa", 2 },
        { "abcdef", -1},
    };

    for (auto [input, result] : cases) {
        if (Solution().maximumLength(input) != result)
            throw "Error answer";
    }

    std::cout << "Pass all cases\n";

    return 0;
}
