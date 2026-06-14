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

int main(int argc, char const *argv[])
{
    Cli cli;

    cli.debug = true;
    cli.cmd = Cli::Create{ .path = "foo.txt", .force = true };

    std::print("cli: {}\n", cli);

    return 0;
}

