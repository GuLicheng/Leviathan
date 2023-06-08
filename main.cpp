#include <leviathan/string/lexical_cast.hpp>
#include <leviathan/string/opt.hpp>


int main(int argc, char const *argv[])
{

    static_assert(!leviathan::string::string_viewable<int>);
    static_assert(leviathan::string::string_viewable<std::string>);

    return 0;
}
