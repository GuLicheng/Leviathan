#pragma once

#include <leviathan/variable.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/variant.hpp>

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <functional>

namespace cpp::config::cli 
{

enum class arg_action
{
    store,        // store value, such as --name bob
    store_multi,  // store multiple values, such as --name bob --name alice
    store_const,  // store a constant value, such as --enable-feature (store true)
    count,        // count occurrences, such as -vvv (count = 3)
    store_true,   // store true if present, such as --enable-feature
    store_false,  // store false if present, such as --disable-feature
    help,         // show help information
    version,      // show version information
};

struct [[=derive::debug]] action_only { };

struct [[=derive::debug]] positional  { int index = 0; };

struct [[=derive::debug]] named 
{
    char short_name = '\0';
    std::string long_name;
};

using arg_kind = std::variant<action_only, positional, named>;

struct arg
{
    // Our parser will search for arguments by their id, so it must be unique.
    std::string id;

    // Help message for the argument, shown in help output.
    std::string help = "No help available";
    
    // The action to perform when this argument is encountered.
    arg_action action = arg_action::store;

    // Whether the argument is required or optional.
    bool is_required = false;

    // The kind of argument: positional, named, or action-only.
    arg_kind kind;
    
    // The constant value to store when arg_action::store_const is used. 
    // This is optional and only relevant for store_const actions.
    std::string const_value;

    // The default value for the argument, if any. This is optional and can be used to provide a fallback.
    std::string default_value;

    // Whether the argument should be hidden from help output. This is optional and defaults to false.
    bool hidden = false;

    // Placeholder for the value in help output, defaults to "VALUE".
    // --output <metavar>
    std::string metavar = "VALUE";  

    // Whether the argument can be specified multiple times.
    bool multiple = false; 

    template <typename T>
    constexpr bool is() const
    {
        return std::holds_alternative<T>(kind);
    }
};



}  // namespace cpp::config::cli


