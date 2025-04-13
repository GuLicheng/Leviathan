#pragma once

#include <leviathan/config_parser/common.hpp>
#include <vector>
#include <string_view>

namespace cpp::config::cmd
{
  
// Command class is a placeholder for command line commands.
class commandline
{
public:

    commandline(int argc, char const* argv[])
    {
        for (int i = 0; i < argc; ++i)
        {
            m_args.emplace_back(argv[i]);
        }
    }

    size_t size() const
    {
        return m_args.size() - 1; //  remove the program name.
    }

    void check_size(size_t count, bool require_equal, std::string message = "Invalid number of arguments") const
    {
        bool result = require_equal ? (size() == count) : (size() >= count);

        if (!result)
        {
            throw std::runtime_error(message);
        }
    }

    // We ignore the first argument which is the program name.
    std::string_view operator[](size_t index) const
    {
        // +1 to skip the program name.
        return index < size() ? m_args[index + 1] : throw std::out_of_range("Index out of range");
    }

    auto begin() const { return m_args.begin(); }

    auto end() const { return m_args.end(); }

private:
    std::vector<std::string_view> m_args;
};

} // namespace cpp::config::cmd

