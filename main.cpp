#include <leviathan/string/lexical_cast.hpp>




int main(int argc, char const *argv[])
{
    std::string s = "12345";

    std::string_view sv = s;

    // constexpr auto ok = std::convertible_to<std::string, std::string_view>;

    return 0;
}
