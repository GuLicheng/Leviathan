/*

    We require any struct to be parsed must be aggregate type, and all its members must be public.
    See Rust.Clap: https://docs.rs/clap/latest/clap/


*/

#pragma once

#include <leviathan/extc++/meta.hpp>
#include <leviathan/config_parser/common.hpp>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <functional>
#include <variant>
#include <memory>
#include <any>

namespace cpp::config::cmd
{


// 前向声明
class arg;
class arg_group;
class command;
using styled_str = std::string;  // 简单起见，styled_str 目前只是 std::string 的别名，未来可以扩展为支持样式的字符串

// ==================== 类型别名 ====================
using str = std::string;                     // 对应 Rust 的 Str
using app_flags = uint64_t;                  // 位标志，对应 Rust 的 AppFlags
using extensions = std::unordered_map<std::string, std::any>;  // 扩展存储

// ==================== command 结构体定义 ====================
struct command {

    // ---------- 基本元信息 ----------
    str m_name;                                      // 命令名称
    std::optional<str> m_long_flag;                  // 长标志（如 --flag）
    std::optional<char> m_short_flag;                // 短标志（如 -f）
    std::optional<std::string> m_display_name;       // 显示名称
    std::optional<std::string> m_bin_name;           // 二进制文件名
    std::optional<str> m_author;                     // 作者
    std::optional<str> m_version;                    // 版本号
    std::optional<str> m_long_version;               // 详细版本信息

    // ---------- 帮助信息 ----------
    std::optional<styled_str> m_about;               // 简短描述
    std::optional<styled_str> m_long_about;          // 详细描述
    std::optional<styled_str> m_before_help;         // 帮助前显示的内容
    std::optional<styled_str> m_before_long_help;    // 帮助前显示的内容（详细版）
    std::optional<styled_str> m_after_help;          // 帮助后显示的内容
    std::optional<styled_str> m_after_long_help;     // 帮助后显示的内容（详细版）

    // ---------- 别名 ----------
    // (name, visible) 可见性表示是否在帮助中显示
    std::vector<std::pair<str, bool>> m_aliases;
    std::vector<std::pair<char, bool>> m_short_flag_aliases;
    std::vector<std::pair<str, bool>> m_long_flag_aliases;

    // ---------- 使用信息 ----------
    std::optional<styled_str> m_usage_str;           // 自定义使用说明
    std::optional<std::string> m_usage_name;         // 使用说明中的命令名
    std::optional<styled_str> m_help_str;            // 自定义帮助文本

    // ---------- 显示与控制 ----------
    std::optional<size_t> m_disp_ord;                // 显示顺序
    app_flags m_settings = 0;                        // 当前命令的设置
    app_flags m_g_settings = 0;                      // 全局设置（从父命令继承）

    // ---------- 核心组件 ----------
    // m_args: 存储所有参数（用哈希表映射参数名到 arg 对象）
    std::unordered_map<std::string, arg> m_args;
    
    // m_subcommands: 子命令列表
    std::vector<command> m_subcommands;
    
    // m_groups: 参数分组（用于互斥等逻辑）
    std::vector<arg_group> m_groups;

    // ---------- 子命令相关 ----------
    std::optional<str> m_current_help_heading;       // 当前帮助标题
    std::optional<size_t> m_current_disp_ord;        // 当前显示顺序
    std::optional<str> m_subcommand_value_name;      // 子命令值的显示名称
    std::optional<str> m_subcommand_heading;         // 子命令部分的标题

    // ---------- 扩展与高级功能 ----------
    // 外部值解析器（对应 Rust 的 ValueParser）
    std::optional<std::function<std::variant<int, double, std::string, bool>(const std::string&)>> m_external_value_parser;
    
    bool m_long_help_exists = false;                 // 是否存在详细帮助

    // 延迟初始化函数（对应 Rust 的 fn(Command) -> Command）
    std::optional<std::function<command(command)>> m_deferred;

    // 扩展存储（用于不稳定特性）
    extensions m_app_ext;
    
    // ---------- 构造函数 ----------

#if 0
    // ---------- 便捷方法（Builder 风格） ----------
    command& version(const str& v) { 
        m_version = v; 
        return *this; 
    }
    
    command& author(const str& a) { 
        m_author = a; 
        return *this; 
    }
    
    command& about(const styled_str& a) { 
        m_about = a; 
        return *this; 
    }

    // 添加参数
    command& arg(arg a) {
        m_args[a.get_name()] = std::move(a);
        return *this;
    }

    // 添加子命令
    command& subcommand(command cmd) {
        m_subcommands.push_back(std::move(cmd));
        return *this;
    }

    // 解析入口
    template<typename Opts>
    Opts parse(int argc, char** argv) {
        // 利用 P2996 反射实现自动解析
        // ...
        return Opts{};
    }

#endif

};

}


