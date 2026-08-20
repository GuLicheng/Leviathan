#pragma once

#include <leviathan/config_parser/cli/arg.hpp>
#include <leviathan/config_parser/cli/parse_result.hpp>

namespace cpp::config::cli
{

class parser
{
    std::vector<std::string> args;

    parse_result result;


public:

    parser(int argc, const char* argv[]) 
        : args(argv, argv + argc) 
    {
    }



};

inline parse_result parse(int argc, const char* argv[])
{
    parser p(argc, argv);
    return p.result;
}

} // namespace cpp::config::cli
