// leviathan/config_parser/cmd/parser.hpp
#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#include "leviathan/config_parser/cmd/command.hpp"
#include "leviathan/config_parser/cmd/arg_matches.hpp"

namespace cpp::config::cmd {

// ============================================================================
// parser - 命令行解析器
// ============================================================================
class parser {
private:
    const command& m_root_cmd;
    arg_matches m_matches;
    int m_position = 1;

    // -------- 参数分类 --------
    enum class arg_type { positional, short_opt, long_opt, none };

    arg_type classify_token(const std::string& token) const {
        if (token.empty()) return arg_type::none;
        if (token == "--") return arg_type::none;  // 停止符
        if (token[0] == '-' && token.size() == 2 && token[1] != '-') {
            return arg_type::short_opt;
        }
        if (token[0] == '-' && token.size() > 2 && token[1] == '-') {
            return arg_type::long_opt;
        }
        return arg_type::positional;
    }

    // -------- 短选项解析 --------
    bool parse_short_arg(const std::string& token) {
        const auto& args = m_root_cmd.args();

        // 连写：-abc
        if (token.size() > 2) {
            for (size_t i = 1; i < token.size(); ++i) {
                char c = token[i];
                const arg* matched = m_root_cmd.find_arg_by_short(c);
                if (!matched) {
                    throw std::runtime_error("Unknown option: -" + std::string(1, c));
                }

                if (matched->takes_value()) {
                    std::string value = token.substr(i + 1);
                    if (value.empty()) {
                        return false;  // 需要下一个 token 作为值
                    }
                    m_matches.set_value(matched->id(), value);
                    return true;
                } else {
                    m_matches.set_flag(matched->id(), true);
                }
            }
            return true;
        }

        // 单个：-c
        char c = token[1];
        const arg* matched = m_root_cmd.find_arg_by_short(c);
        if (!matched) {
            throw std::runtime_error("Unknown option: " + token);
        }

        if (matched->takes_value()) {
            return false;
        } else {
            m_matches.set_flag(matched->id(), true);
            return true;
        }
    }

    // -------- 长选项解析 --------
    bool parse_long_arg(const std::string& token) {
        std::string name;
        std::string value;

        size_t eq_pos = token.find('=');
        if (eq_pos != std::string::npos) {
            name = token.substr(2, eq_pos - 2);
            value = token.substr(eq_pos + 1);
        } else {
            name = token.substr(2);
        }

        const arg* matched = m_root_cmd.find_arg_by_long(name);
        if (!matched) {
            throw std::runtime_error("Unknown option: " + token);
        }

        if (matched->takes_value()) {
            if (!value.empty()) {
                m_matches.set_value(matched->id(), value);
                return true;
            }
            return false;
        } else {
            m_matches.set_flag(matched->id(), true);
            return true;
        }
    }

    // -------- 位置参数解析 --------
    void parse_positional(const std::string& token) {
        const arg* matched = m_root_cmd.find_arg_by_position(m_position);
        if (!matched) {
            throw std::runtime_error("Unexpected positional argument: " + token);
        }
        m_matches.set_value(matched->id(), token);
        m_position++;
    }

    // -------- 子命令解析 --------
    bool try_parse_subcommand(const std::string& token, int& i, int argc, char** argv) {
        const command* subcmd = m_root_cmd.find_subcommand(token);
        if (!subcmd) {
            return false;
        }

        // 构建子命令的参数列表
        std::vector<const char*> sub_argv;
        for (int j = i + 1; j < argc; ++j) {
            sub_argv.push_back(argv[j]);
        }

        // 递归解析子命令
        parser sub_parser(*subcmd);
        arg_matches sub_matches = sub_parser.parse(
            static_cast<int>(sub_argv.size()),
            const_cast<char**>(sub_argv.data())
        );

        m_matches.set_subcommand(token);
        m_matches.set_subcommand_matches(std::move(sub_matches));
        i = argc;  // 跳过所有剩余参数
        return true;
    }

    // -------- 主解析循环 --------
    void parse_args(int argc, char** argv) {
        bool stop_parsing = false;

        for (int i = 1; i < argc; ++i) {
            std::string token = argv[i];

            // 检查子命令（只在不以 '-' 开头且未停止时）
            if (!stop_parsing && token[0] != '-') {
                if (try_parse_subcommand(token, i, argc, argv)) {
                    continue;
                }
            }

            if (token == "--") {
                stop_parsing = true;
                continue;
            }

            if (stop_parsing) {
                parse_positional(token);
                continue;
            }

            arg_type type = classify_token(token);

            switch (type) {
                case arg_type::short_opt: {
                    bool done = parse_short_arg(token);
                    if (!done) {
                        if (i + 1 >= argc) {
                            throw std::runtime_error("Option " + token + " requires a value");
                        }
                        char c = token[1];
                        const arg* matched = m_root_cmd.find_arg_by_short(c);
                        if (matched) {
                            ++i;
                            m_matches.set_value(matched->id(), argv[i]);
                        }
                    }
                    break;
                }
                case arg_type::long_opt: {
                    bool done = parse_long_arg(token);
                    if (!done) {
                        if (i + 1 >= argc) {
                            throw std::runtime_error("Option " + token + " requires a value");
                        }
                        std::string name = token.substr(2);
                        const arg* matched = m_root_cmd.find_arg_by_long(name);
                        if (matched) {
                            ++i;
                            m_matches.set_value(matched->id(), argv[i]);
                        }
                    }
                    break;
                }
                case arg_type::positional: {
                    parse_positional(token);
                    break;
                }
                default:
                    throw std::runtime_error("Unexpected token: " + token);
            }
        }
    }

    // -------- 验证 --------
    void validate_required() {
        const auto& args = m_root_cmd.args();
        for (const auto& a : args) {
            if (a.is_required() && !m_matches.is_present(a.id())) {
                throw std::runtime_error("Required argument missing: " + a.id());
            }
        }
    }

public:
    // -------- 构造函数 --------
    explicit parser(const command& cmd) : m_root_cmd(cmd) {}

    // -------- 解析入口 --------
    arg_matches parse(int argc, char** argv) {
        if (argc < 1) {
            throw std::runtime_error("No arguments provided");
        }

        // 先检查是否请求帮助或版本（在 command::parse_or_help 中处理）
        // 这里只做纯粹解析

        parse_args(argc, argv);
        validate_required();
        return std::move(m_matches);
    }
};

} // namespace cpp::config::cmd