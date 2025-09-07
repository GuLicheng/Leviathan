#include <leviathan/nom/nom.hpp>
#include <leviathan/meta/type.hpp>
#include <print>
#include <leviathan/extc++/ranges.hpp>
#include <filesystem>
#include <strstream>
#include <string_view>
#include <sstream>

struct Color
{
    int r, g, b;
};

int main(int argc, char const *argv[])
{
    Color color = std::make_from_tuple<Color>(std::tuple{255, 0, 255});


    return 0;
}
