#pragma once

#include <leviathan/variable.hpp>
#include <leviathan/extc++/format.hpp>

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <functional>

namespace cpp::config::cli 
{

enum class [[=derive::debug]] arg_action
{
    set,          // 存储一个值
    set_true,     // 布尔标志（出现即为 true）
    set_false,    // 反向布尔标志（出现即为 false）
    append,       // 追加到向量（可多次出现）
    count,        // 计数器（统计出现次数）
    help,         // 显示帮助信息
    version,      // 显示版本信息
};

struct [[=derive::debug]] arg
{
    std::string id;
    std::string help = "No help available";
    arg_action action = arg_action::set;
    bool required = false;
    bool takes_value = true;

    struct action_only { };

    // 位置参数
    struct positional {
        int index = 0;
    };

    // 命名参数
    struct named {
        char short_name = '\0';
        std::string long_name;
    };

    using kind = std::variant<action_only, positional, named>;

    // kind arg_kind;

    // 预留字段
    // std::optional<std::string> default_value;
    // bool hidden = false;
    // std::vector<std::string> aliases;
    // std::vector<std::string> possible_values;
    // std::vector<std::string> conflicts_with;
    // std::vector<std::string> requires_fields;
};


}  // namespace cpp::config::cli


