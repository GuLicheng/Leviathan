#include <leviathan/config_parser/encode.hpp>



int main(int argc, char const *argv[])
{
    static_assert(leviathan::config::digit_values[0] == -1);
    static_assert(leviathan::config::digit_values['0'] == 0);
    static_assert(leviathan::config::digit_values['a'] == 10);
    static_assert(leviathan::config::digit_values['A'] == 10);

    return 0;
}
