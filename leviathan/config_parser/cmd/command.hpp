// leviathan/config_parser/cmd/command.hpp
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include "leviathan/config_parser/cmd/arg.hpp"
#include "leviathan/config_parser/cmd/arg_matches.hpp"

namespace cpp::config::cmd {

// 前向声明
class parser;

// ============================================================================
// command - 命令定义
// ============================================================================
class command {
public:
    // -------- 构造函数 --------
    command() = default;
    explicit command(std::string name) : m_name(std::move(name)) {}

    // -------- Builder 方法 --------
    command& name(std::string n) { m_name = std::move(n); return *this; }
    command& description(std::string d) { m_description = std::move(d); return *this; }
    command& version(std::string v) { m_version = std::move(v); return *this; }
    command& author(std::string a) { m_author = std::move(a); return *this; }

    command& arg(arg a) {
        // 检查 id 是否已存在
        for (const auto& existing : m_args) {
            if (existing.id() == a.id()) {
                throw std::runtime_error("Duplicate argument id: " + a.id());
            }
        }
        m_args.push_back(std::move(a));
        return *this;
    }

    command& args(const std::vector<arg>& args) {
        for (const auto& a : args) {
            arg(a);  // 复用 arg 方法，包含重复检查
        }
        return *this;
    }

    command& subcommand(command cmd) {
        // 检查子命令名是否已存在
        for (const auto& existing : m_subcommands) {
            if (existing.name() == cmd.name()) {
                throw std::runtime_error("Duplicate subcommand: " + cmd.name());
            }
        }
        m_subcommands.push_back(std::move(cmd));
        return *this;
    }

    // -------- 解析入口 --------
    arg_matches parse(int argc, char** argv) const;

    // 解析并处理帮助/版本
    arg_matches parse_or_help(int argc, char** argv) const;

    // -------- 查询方法 --------
    const std::string& name() const { return m_name; }
    const std::string& description() const { return m_description; }
    const std::string& version() const { return m_version; }
    const std::string& author() const { return m_author; }
    const std::vector<arg>& arguments() const { return m_args; }
    const std::vector<command>& subcommands() const { return m_subcommands; }

    // -------- 辅助查询 --------
    const arg* find_arg_by_id(const std::string& id) const {
        for (const auto& a : m_args) {
            if (a.id() == id) {
                return &a;
            }
        }
        return nullptr;
    }

    const arg* find_arg_by_short(char c) const {
        for (const auto& a : m_args) {
            if (a.is_named() && a.short_name() == c) {
                return &a;
            }
        }
        return nullptr;
    }

    const arg* find_arg_by_long(const std::string& name) const {
        for (const auto& a : m_args) {
            if (a.is_named() && a.long_name() == name) {
                return &a;
            }
        }
        return nullptr;
    }

    const arg* find_arg_by_position(int pos) const {
        for (const auto& a : m_args) {
            if (a.is_positional() && a.index() == pos) {
                return &a;
            }
        }
        return nullptr;
    }

    const command* find_subcommand(const std::string& name) const {
        for (const auto& cmd : m_subcommands) {
            if (cmd.name() == name) {
                return &cmd;
            }
        }
        return nullptr;
    }

    // -------- 帮助信息生成 --------
    std::string generate_help() const {
        std::stringstream ss;
        ss << "Usage: " << m_name;

        bool has_options = false;
        for (const auto& a : m_args) {
            if (a.is_named()) {
                if (!has_options) {
                    ss << " [OPTIONS]";
                    has_options = true;
                }
            }
        }

        for (const auto& a : m_args) {
            if (a.is_positional()) {
                ss << (a.is_required() ? " <" : " [")
                   << a.id()
                   << (a.is_required() ? ">" : "]");
            }
        }

        if (!m_subcommands.empty()) {
            ss << " [SUBCOMMAND]";
        }

        ss << "\n\n";

        if (!m_description.empty()) {
            ss << m_description << "\n\n";
        }

        // Options 部分
        has_options = false;
        for (const auto& a : m_args) {
            if (a.is_named() && !a.is_hidden()) {
                if (!has_options) {
                    ss << "Options:\n";
                    has_options = true;
                }
                ss << "  ";
                if (a.short_name() != '\0') {
                    ss << "-" << a.short_name();
                    if (!a.long_name().empty()) {
                        ss << ", ";
                    }
                }
                if (!a.long_name().empty()) {
                    ss << "--" << a.long_name();
                }
                if (a.takes_value()) {
                    ss << " <" << a.id() << ">";
                }
                ss << "\n";
                ss << "      " << a.help() << "\n";
            }
        }

        // 位置参数部分
        bool has_positional = false;
        for (const auto& a : m_args) {
            if (a.is_positional() && !a.is_hidden()) {
                if (!has_positional) {
                    if (has_options) ss << "\n";
                    ss << "Arguments:\n";
                    has_positional = true;
                }
                ss << "  " << (a.is_required() ? "<" : "[")
                   << a.id()
                   << (a.is_required() ? ">" : "]")
                   << "  " << a.help() << "\n";
            }
        }

        // 子命令部分
        if (!m_subcommands.empty()) {
            if (has_options || has_positional) ss << "\n";
            ss << "Subcommands:\n";
            for (const auto& cmd : m_subcommands) {
                ss << "  " << cmd.name()
                   << (cmd.description().empty() ? "" : "  " + cmd.description())
                   << "\n";
            }
        }

        return ss.str();
    }

    void print_help() const {
        std::cout << generate_help();
    }

private:
    std::string m_name;
    std::string m_description;
    std::string m_version;
    std::string m_author;

    std::vector<arg> m_args;
    std::vector<command> m_subcommands;
};

} // namespace cpp::config::cmd

// 包含 parser 实现（因为 command::parse 需要 parser 定义）
#include "leviathan/config_parser/cmd/parser.hpp"

namespace cpp::config::cmd {

// -------- command::parse 实现 --------
inline arg_matches command::parse(int argc, char** argv) const {
    parser p(*this);
    return p.parse(argc, argv);
}

inline arg_matches command::parse_or_help(int argc, char** argv) const {
    // 检查是否请求帮助或版本
    for (int i = 1; i < argc; ++i) {
        std::string arg_str = argv[i];
        if (arg_str == "--help" || arg_str == "-h") {
            print_help();
            return arg_matches{};  // 返回空结果
        }
        if (arg_str == "--version" || arg_str == "-V") {
            if (!m_version.empty()) {
                std::cout << m_name << " " << m_version << "\n";
            }
            return arg_matches{};
        }
    }

    try {
        return parse(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << "\n";
        print_help();
        throw;  // 重新抛出
    }
}

} // namespace cpp::config::cmd