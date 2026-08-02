#include <leviathan/config_parser/cli/arg.hpp>
#include <print>

namespace cli = cpp::config::cli;

int main()
{
    auto arg = cli::arg{ .id = "verbose", .help = "Enable verbose output", .action = cli::arg_action::set_true };

    std::println("Argument: {}", arg);
}
