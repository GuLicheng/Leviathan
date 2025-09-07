#include <leviathan/nom/nom.hpp>
#include <leviathan/meta/type.hpp>
#include <print>
#include <leviathan/extc++/ranges.hpp>
#include <filesystem>
#include <strstream>
#include <sstream>

int main(int argc, char const *argv[])
{
    auto s = "ͽ"; 

    // const char chars[] = { char('0x03'), char('0x7D'), 0 };
    std::string chars;

    chars.append({ char(0xCD), char(0xBD) });
    
    std::views::filter(chars, [] (char c) { return c != 0; });

    std::vector<int>().append_range({1,2,3,4,5});
    
    auto iter = std::filesystem::directory_iterator("D:/Library/Leviathan");

    std::istream_iterator<char> it(std::cin), end;

    std::println("{}", chars);
    std::println("{}|{}", (unsigned char)s[0], (unsigned char)s[1]);

    return 0;
}
