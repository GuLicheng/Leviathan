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

class [[=cpp::derive::tuple_like]] Point2D : public cpp::tuple_get_interface
{
    [[=cpp::refl::skip]]
    int ID;
    int X;
    int Y;
public:
    Point2D(int x, int y) : X(x), Y(y) {}
};

int main(int argc, char const *argv[])
{
    Cli cli;

    cli.debug = true;
    cli.cmd = Cli::Create{ .path = "foo.txt", .force = true };

    std::print("cli: {}\n", cli);

    std::println("Point2D is tuple-like: {}", std::tuple_size_v<Point2D>);

    auto [x, y] = Point2D{1, 2};
    std::println("Point2D: ({}, {})", x, y);

    return 0;
}

