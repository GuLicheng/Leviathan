#include <iostream>
#include <string>
#include <regex>
#include <lv_cpp/meta/template_info.hpp>

inline static const std::regex Pattern{ "-*(.*)=(.*)" };

int main() 
{
    std::string_view strings[] = {
        "-std=c++",
        "--v=1.4.4",
        "create",
        "-err="
    };

    for (auto& string : strings) {
        std::match_results<std::string_view::iterator> ss;
        std::cout << "Length = " << string.size() << "\n";
        if (std::regex_match(string.begin(), string.end(), ss, Pattern)) {
            for (auto res : ss)
                std::cout << '(' << res << ')' << '\n';
            std::cout << "====================================\n";
        }
    }
}