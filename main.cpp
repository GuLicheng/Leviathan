#include <variant>
#include <print>
#include <vector>
#include <algorithm>
#include <ranges>
#include <string>
#include <iostream>
#include <functional>

#include <leviathan/photo/bmp.hpp>
#include <leviathan/photo/png.hpp>
#include <leviathan/photo/writer.hpp>

consteval uint32_t chunk_type(std::string_view sv)
{
    assert(sv.size() == 4);
    return (static_cast<uint32_t>(sv[0]) << 24) |
           (static_cast<uint32_t>(sv[1]) << 16) |
           (static_cast<uint32_t>(sv[2]) << 8)  |
           (static_cast<uint32_t>(sv[3]));
}

int main(int argc, char const *argv[])
{
    cpp::image::png::read_png_file(R"(D:\Library\Leviathan\lena.png)");
    return 0;
}

