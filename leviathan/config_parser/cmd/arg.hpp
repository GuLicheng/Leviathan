#pragma once

#include <leviathan/variable.hpp>

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <functional>

namespace cpp::config::cmd {

// ============================================================================
// arg_action - 参数行为枚举
// ============================================================================
enum class arg_action 
{
    set,          // 存储一个值
    set_true,     // 布尔标志（出现即为 true）
    set_false,    // 反向布尔标志（出现即为 false）
    append,       // 追加到向量（可多次出现）
    count,        // 计数器（统计出现次数）
    help,         // 显示帮助信息
    version,      // 显示版本信息
};

// ============================================================================
// arg - 单个参数定义
// ============================================================================
class arg {

    // -------- 内部类型 --------
    struct [[=derive::debug]] action_only { };

    struct [[=derive::debug]] positional {
        int index = 0;
    };
    struct [[=derive::debug]] named {
        char short_name = '\0';
        std::string long_name;
    };
    
    using kind = std::variant<action_only, positional, named>;

public:
    // -------- 构造函数 --------
    arg() = default;
    explicit arg(std::string id) : m_id(std::move(id)) {}

    // -------- Builder 方法 --------
    arg& id(std::string id) { m_id = std::move(id); return *this; }
    arg& help(std::string h) { m_help = std::move(h); return *this; }
    arg& action(arg_action a) { m_action = a; return *this; }
    arg& required(bool r) { m_required = r; return *this; }
    arg& takes_value(bool tv) { m_takes_value = tv; return *this; }

    // -------- 位置参数 --------
    arg& positional(int index) {
        m_kind.emplace<1>(index);
        return *this;
    }

    // -------- 命名参数 --------
    arg& named(char short_name, const std::string& long_name = "") {
        m_kind.emplace<2>(short_name, long_name);
        return *this;
    }

    arg& short_name(char c) {
        if (auto* n = std::get_if<2>(&m_kind)) {
            n->short_name = c;
        } else {
            m_kind.emplace<2>(c, "");
        }
        return *this;
    }

    arg& long_name(const std::string& name) {
        if (auto* n = std::get_if<2>(&m_kind)) {
            n->long_name = name;
        } else {
            m_kind.emplace<2>('\0', name);
        }
        return *this;
    }

    // -------- 预留字段（未来扩展） --------
    arg& default_value(std::string dv) { m_default_value = std::move(dv); return *this; }
    arg& hidden(bool h) { m_hidden = h; return *this; }

    // -------- 查询方法 --------
    const std::string& id() const { return m_id; }
    const std::string& help() const { return m_help; }
    arg_action get_action() const { return m_action; }
    bool is_required() const { return m_required; }
    bool takes_value() const { return m_takes_value; }

    bool is_positional() const {
        return m_kind.index() == 1;
    }

    bool is_named() const {
        return m_kind.index() == 2;
    }

    bool is_action_only() const {
        return m_kind.index() == 0;
    }

    int index() const {
        return m_kind.index() == 1 ? std::get<1>(m_kind).index : -1;
    }

    char short_name() const {
        return m_kind.index() == 2 ? std::get<2>(m_kind).short_name : '\0';
    }

    const std::string& long_name() const {
        static const std::string empty_string;
        return m_kind.index() == 2 ? std::get<2>(m_kind).long_name : empty_string;
    }

    // -------- 预留字段的 getter --------
    bool has_default_value() const { return m_default_value.has_value(); }
    const std::optional<std::string>& default_value() const { return m_default_value; }
    bool is_hidden() const { return m_hidden; }

private:


    // -------- 成员变量 --------
    std::string m_id;
    std::string m_help = "No help available";
    arg_action m_action = arg_action::set;
    bool m_required = false;
    bool m_takes_value = true;

    kind m_kind;

    // 预留字段
    std::optional<std::string> m_default_value;
    bool m_hidden = false;
    std::vector<std::string> m_aliases;
    std::vector<std::string> m_possible_values;
    std::vector<std::string> m_conflicts_with;
    std::vector<std::string> m_requires;
};

} // namespace cpp::config::cmd