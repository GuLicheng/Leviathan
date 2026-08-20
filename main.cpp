#include <leviathan/config_parser/cli/arg.hpp>
#include <leviathan/config_parser/cli/command.hpp>
#include <print>

namespace cli = cpp::config::cli;

void dump_command(const cli::command& cmd)
{
    std::print("Usage: {} <FILE> [OPTIONS] [SUBCOMMANDS] [ARGS...]\n", cmd.name);

    std::print("\nDescription:\n  {}\n", cmd.description);

    if (!cmd.args.empty())
    {
        std::print("\nArguments:\n");
        for (const auto& arg : cmd.args)
        {
            std::string kind_str;
            if (std::holds_alternative<cli::positional>(arg.kind))
            {
                kind_str = "Positional";
            }
            else if (std::holds_alternative<cli::named>(arg.kind))
            {
                kind_str = "Named";
            }
            else if (std::holds_alternative<cli::action_only>(arg.kind))
            {
                kind_str = "Action-only";
            }

            std::print("  {}: {} ({})\n", arg.id, arg.help, kind_str);
        }
    }

    if (!cmd.subcommands.empty())
    {
        std::print("\nSubcommands:\n");
        for (const auto& sub : cmd.subcommands)
        {
            std::print("  {}: {}\n", sub.name, sub.description);
        }
    }

    
}

int main()
{
    auto arg = cli::arg { 
        .id = "verbose", 
        .help = "Enable verbose output", 
        .action = cli::arg_action::store_true, 
        .kind = cli::named{ .short_name = 'v', .long_name = "verbose" } 
    };

    auto cmd = cli::command { 
        .name = "xor", 
        .description = "My Application", 
        .hidden = false 
    };

    cmd.add_argument(cli::arg { 
        .id = "input", 
        .help = "Input file", 
        .action = cli::arg_action::store, 
        .kind = cli::positional{ .index = 0 } 
    });

    dump_command(cmd);
}
