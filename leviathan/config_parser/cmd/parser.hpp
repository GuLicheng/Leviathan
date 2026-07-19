#pragma once

#include <span>

namespace cpp::config::cmd
{

/*
    input: myapp --config config.toml input.txt

    start:                 values_done
    --config:              opt(ConfigId)    // waiting option value
    config.toml:           values_done      // read option value
    input.txt:             pos(InputId)     // waiting positional argument
    end:                   values_done      // parsing complete
*/
enum parse_state
{
    values_done,    // 解析完成
    opt,            // 解析到选项
    pos,            // 解析到位置参数
};
    

class parser
{
    std::span<const char* const> m_args;

    void parse_location();

    void parse_option();

public:

    parser(int argc, const char* const* argv) : m_args(argv, argc) {}

    


};


}
