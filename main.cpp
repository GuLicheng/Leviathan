#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/variant.hpp>
#include <utility>
#include <mdspan>
#include <iostream>
#include <variant>
#include <print>
#include <tuple>

inline constexpr struct { } subcommand;
inline constexpr struct { } global;
inline constexpr struct { } shortname;

struct [[=cpp::derive::debug]] Cli {

    [[=global, =shortname]]
    bool debug;
    
    struct [[=cpp::derive::debug]] Create {
        std::string path;
        bool force;
    };

    struct [[=cpp::derive::debug]] Delete {
        std::string path;
    };

    struct [[=cpp::derive::debug]] List { };

    using Commands = std::variant<std::monostate, Create, Delete, List>;

    [[=subcommand]]
    Commands cmd;
};

namespace cxx
{

class [[=cpp::derive::tuple_like]] Point : public cpp::tuple_get_interface
{
    [[=cpp::refl::tuple_element]]
    int X;

    [[=cpp::refl::tuple_element]]
    double Y;

    [[=cpp::refl::tuple_element]]
    std::string Z = "hello";  

public:

    Point(int x, int y) : X(x), Y(y) { }
};

} // namespace cxx

int main(int argc, char const *argv[])
{
    using namespace cxx;

    Cli cli;

    cli.debug = true;
    cli.cmd = Cli::Create{ .path = "foo.txt", .force = true };

    std::print("cli: {}\n", cli);
    Point p{1, 2};

    std::print("point: x: {}\n", p.[:cpp::refl::member_named<Point>("X"):]);

    std::println("point as tuple: {}", std::tuple_size_v<Point>);

    auto&& [x, y, z] = p;
    std::print("point: x: {}, y: {}, z: {}\n", x, y, z);

    return 0;
}

