#include <leviathan/nom/nom.hpp>
#include <leviathan/meta/type.hpp>
#include <print>

int main(int argc, char const *argv[])
{
    auto s = "ͽ"; 

    // const char chars[] = { char('0x03'), char('0x7D'), 0 };
    std::string chars;

    chars.append({ char(0xCD), char(0xBD) });

    std::println("{}", chars);
    std::println("{}|{}", (unsigned char)s[0], (unsigned char)s[1]);

    return 0;
}
