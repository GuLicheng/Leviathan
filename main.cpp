#include <variant>
#include <print>
#include <vector>
#include <algorithm>
#include <ranges>
#include <string>
#include <iostream>
#include <span>
#include <functional>

#include <leviathan/photo/bmp.hpp>
#include <leviathan/photo/png.hpp>
#include <leviathan/photo/writer.hpp>
#include <leviathan/extc++/functional.hpp>
#include <leviathan/algorithm/lz77.hpp>

using namespace cpp::algorithm;

void TestLZ77(std::string data, int windowSize, int lookaheadSize)
{
    auto res = lz77_compress(data.begin(), data.end(), windowSize, lookaheadSize);

    for (const auto& token : res) {
        std::println("({}, {}, {})", token.offset, token.length, *token.next);
    }


    std::println("Decompress:");
    auto decompressed = lz77_decompress(res);
    for (const auto& ch : decompressed) {
        std::print("{}", ch);
    }
    std::println("\n==================================");
}

int main(int argc, char const *argv[])
{
    TestLZ77("aacaacabcabaaac", 6, 4);
    TestLZ77("AABCBBABC", 5, 3);
    TestLZ77("BBBBBBBBBBBBBBBBBBBBBBBBBA", 1024, 100);

    std::string code = "we will we will r u";

    auto res = huffman(code.begin(), code.end());
    res.show();

    std::println("Encoded: {}", res.encode(code.begin(), code.end()).size());

    return 0;
}

