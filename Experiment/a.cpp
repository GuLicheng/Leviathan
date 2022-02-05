#include <lv_cpp/string/opt.hpp>
#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/io/console.hpp>
#include <lv_cpp/format_extend.hpp>
#include <iostream>

int main()
{
    using namespace leviathan;
    using namespace std::literals;
    std::string s1 = "Hello";
    // for std::string("!"), it's better to use "!" instead.
    lazy_string_concat_helper concat { s1, " World ", std::string("!"), std::string_view(" C++ ") };
    auto concat2 = concat.cat_with(" C# ").cat_with(std::string(" C ")).cat_with(s1).cat_with(" Python "sv);
    std::cout << concat2.to_string() << '\n'; 

    auto empty_str = " "s;

    console::write_line(trim("std::string"s));
    console::write_line(trim("std::string"sv));
    console::write_line(trim(empty_str));

}
