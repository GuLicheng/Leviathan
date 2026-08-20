#pragma once

#include <vector>
#include <unordered_map>

namespace cpp::config::cli 
{

struct arg_result
{
    // The unique identifier of the argument that this result corresponds to. 
    // This should match the id of an arg defined in the command.
    std::string id;

    // The values associated with the argument, if any. 
    // For example, for a named argument like --name bob, 
    // this would contain "bob". For a positional argument, 
    // it would contain the value provided in the command line.
    std::vector<std::string> values;

    // Whether the argument was present in the parsed input.
    bool present = false;
};

class parse_result
{
    // The command that was matched during parsing.
    std::vector<arg_result> command_path;   

    // Key: arg::id
    std::unordered_map<std::string, arg_result> args;

    std::vector<std::string> unparsed_args;
};
    
}  // namespace cpp::config::cli
