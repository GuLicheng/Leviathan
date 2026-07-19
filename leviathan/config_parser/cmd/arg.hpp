#pragma once

#include <string>
#include <vector>

namespace cpp::config::cmd
{

enum class action
{
    set,
    append,
    set_true,
    set_false,
    count,
    help,
    help_short,
    help_long,
    version,
};


class arg
{
    std::string m_id;
    std::string m_help;
    action m_action;
    int m_index;
    std::vector<std::string> m_names;  // store all names (short and long) for the argument
};

}
