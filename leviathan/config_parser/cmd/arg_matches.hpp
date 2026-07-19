// leviathan/config_parser/cmd/arg_matches.hpp
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace cpp::config::cmd {

// ============================================================================
// arg_matches - 解析结果存储
// ============================================================================
class arg_matches {
public:
    // -------- 设置值 --------
    void set_value(const std::string& id, const std::string& value) {
        m_values[id] = value;
    }

    void set_flag(const std::string& id, bool value) {
        m_flags[id] = value;
    }

    void append_value(const std::string& id, const std::string& value) {
        m_multi_values[id].push_back(value);
    }

    // 设置子命令信息
    void set_subcommand(const std::string& name) {
        m_subcommand = name;
    }
    void set_subcommand_matches(arg_matches matches) {
        m_subcommand_matches = std::move(matches);
    }

    // -------- 查询值 --------
    template<typename T>
    std::optional<T> get_one(const std::string& id) const {
        // 检查 flag
        auto fit = m_flags.find(id);
        if (fit != m_flags.end()) {
            if constexpr (std::is_same_v<T, bool>) {
                return fit->second;
            }
        }

        // 检查 value
        auto vit = m_values.find(id);
        if (vit != m_values.end()) {
            if constexpr (std::is_same_v<T, std::string>) {
                return vit->second;
            } else if constexpr (std::is_same_v<T, int>) {
                try {
                    return std::stoi(vit->second);
                } catch (const std::exception&) {
                    return std::nullopt;
                }
            } else if constexpr (std::is_same_v<T, double>) {
                try {
                    return std::stod(vit->second);
                } catch (const std::exception&) {
                    return std::nullopt;
                }
            } else if constexpr (std::is_same_v<T, const char*>) {
                return vit->second.c_str();
            }
        }

        return std::nullopt;
    }

    template<typename T>
    std::vector<T> get_many(const std::string& id) const {
        auto it = m_multi_values.find(id);
        if (it == m_multi_values.end()) {
            return {};
        }

        std::vector<T> result;
        for (const auto& s : it->second) {
            if constexpr (std::is_same_v<T, std::string>) {
                result.push_back(s);
            } else if constexpr (std::is_same_v<T, int>) {
                result.push_back(std::stoi(s));
            } else if constexpr (std::is_same_v<T, double>) {
                result.push_back(std::stod(s));
            } else if constexpr (std::is_same_v<T, const char*>) {
                result.push_back(s.c_str());
            }
        }
        return result;
    }

    bool is_present(const std::string& id) const {
        return m_values.find(id) != m_values.end() ||
               m_flags.find(id) != m_flags.end() ||
               m_multi_values.find(id) != m_multi_values.end();
    }

    // -------- 子命令查询 --------
    bool has_subcommand() const { return m_subcommand.has_value(); }
    const std::optional<std::string>& subcommand() const { return m_subcommand; }
    const std::optional<arg_matches>& subcommand_matches() const { return m_subcommand_matches; }

    // -------- 调试输出 --------
    void print_all() const {
        std::cout << "=== Parsed Results ===\n";
        for (const auto& [id, val] : m_values) {
            std::cout << id << " = " << val << "\n";
        }
        for (const auto& [id, flag] : m_flags) {
            std::cout << id << " = " << (flag ? "true" : "false") << "\n";
        }
        for (const auto& [id, vals] : m_multi_values) {
            std::cout << id << " = [";
            for (size_t i = 0; i < vals.size(); ++i) {
                std::cout << vals[i];
                if (i < vals.size() - 1) std::cout << ", ";
            }
            std::cout << "]\n";
        }
        if (m_subcommand.has_value()) {
            std::cout << "subcommand = " << *m_subcommand << "\n";
        }
    }

private:
    std::unordered_map<std::string, std::string> m_values;
    std::unordered_map<std::string, bool> m_flags;
    std::unordered_map<std::string, std::vector<std::string>> m_multi_values;
    std::optional<std::string> m_subcommand;
    std::optional<arg_matches> m_subcommand_matches;
};

} // namespace cpp::config::cmd