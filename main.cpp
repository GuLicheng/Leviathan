#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>

class [[=cpp::derive::tuple_like]] Point : public cpp::tuple_get_interface
{
    int X;
    int Y;
};


int main(int argc, char const *argv[])
{
    constexpr auto size = std::meta::tuple_size(^^Point);
    static_assert(size == 2, "Point should have 2 members");
    return 0;
}
