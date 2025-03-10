#include <leviathan/encode.hpp>
#include <leviathan/print.hpp>

int main(int argc, char const *argv[])
{
    leviathan::encoding::UTF8<char> utf8;

    const char* str = "我爱北京天安门";

    std::string_view sv = "我爱北京天安门!";

    Console::WriteLine("UTF-8: {}", utf8.length(str, ::strlen(str)));
    Console::WriteLine("UTF-8: {}", utf8.length(sv));

    leviathan::encoding::UTF16<char, std::endian::little> utf16;

    uint32_t codepoint = 0x0024;

    char buffer[2];

    char* pos = buffer;

    pos = utf16.from_unicode(pos, codepoint);

    for (auto c : buffer)
    {
        Console::Write("{:02X} ", c);
    }

    return 0;
}


