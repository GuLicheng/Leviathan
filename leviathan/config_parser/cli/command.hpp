
#pragma once

#include <leviathan/config_parser/cli/arg.hpp>

#include <string>
#include <vector>

namespace cpp::config::cli 
{

struct command
{
    // The name of the command, used to invoke it from the CLI.
    // For root commands, this is typically the name of the executable.
    std::string name;

    // A brief description of what the command does, shown in help output.
    std::string description;

    // Subcommands that can be invoked under this command. This allows for nested command structures.
    std::vector<command> subcommands;

    // Arguments owned by this command
    std::vector<arg> args;

    // Whether the command should be hidden from help output.
    bool hidden = false;

    // Convenience builder‑style helpers (optional, public members still usable directly)
    command& add_arg(arg a)
    {
        args.push_back(std::move(a));
        return *this;
    }

    command& add_subcommand(command sub)
    {
        subcommands.push_back(std::move(sub));
        return *this;
    }
};

}  // namespace cpp::config::cli