// main.cpp
#include <iostream>

#include "leviathan/config_parser/cmd/cmd.hpp"

using namespace cpp::config::cmd;

int main(int argc, char** argv) {
    try {
        auto matches = command("myapp")
            .description("一个示例 CLI 程序")
            .version("1.0.0")
            .author("Your Name")
            .arg(arg("input")
                .help("输入文件路径")
                .positional(1)
                .required(true))
            .arg(arg("output")
                .help("输出文件路径")
                .positional(2))
            .arg(arg("config")
                .help("配置文件路径")
                .named('c', "config")
                .takes_value(true))
            .arg(arg("verbose")
                .help("启用详细输出")
                .named('v', "verbose")
                .takes_value(false))
            .arg(arg("count")
                .help("重复次数")
                .named('n', "count")
                .takes_value(true))
            .subcommand(
                command("init")
                    .description("初始化项目")
                    .arg(arg("name")
                        .help("项目名称")
                        .positional(1)
                        .required(true))
            )
            .parse_or_help(argc, argv);

        // 检查是否有子命令
        if (matches.has_subcommand()) {
            auto subcmd = matches.subcommand();
            if (subcmd && *subcmd == "init") {
                auto sub_matches = matches.subcommand_matches();
                if (sub_matches) {
                    auto name = sub_matches->get_one<std::string>("name");
                    if (name) {
                        std::cout << "初始化项目: " << *name << "\n";
                    }
                }
            }
            return 0;
        }

        // 获取结果
        auto input = matches.get_one<std::string>("input");
        auto output = matches.get_one<std::string>("output");
        auto config = matches.get_one<std::string>("config");
        auto verbose = matches.get_one<bool>("verbose");
        auto count = matches.get_one<int>("count");

        if (input) {
            std::cout << "输入文件: " << *input << "\n";
        }
        if (output) {
            std::cout << "输出文件: " << *output << "\n";
        }
        if (config) {
            std::cout << "配置文件: " << *config << "\n";
        }
        if (verbose && *verbose) {
            std::cout << "详细模式: 启用\n";
        }
        if (count) {
            std::cout << "重复次数: " << *count << "\n";
        }

        matches.print_all();

    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << "\n";
        return 1;
    }

    return 0;
}