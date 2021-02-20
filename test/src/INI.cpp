#include <lv_cpp/config_parser/INI.hpp>
#include <lv_cpp/io/console.hpp>
#include <iostream>

constexpr const char* path = R"(D:\Library\Leviathan\data.ini)";

constexpr const char* setting = R"(D:\GraphTheoryCode\Template\Tiamat\configuration\config.ini)";

void test_for_read();
void test_for_get();
void test_for_write();
void test();

int main()
{
    // test();
    test_for_write();
    test_for_read();
    // test_for_get();
    std::cout << "Ok\n";
}

void test()
{
    leviathan::ini::INI_handler reader;
    reader.load(setting);
    for (const auto& [k, v] : reader.get_items())
    {
        std::cout << k << v << std::endl;
        // console::write_line(__pair); 屑
    }
}

void test_for_read()
{
    // std::function<void(int, int)> a = std::swap<int, int>;
    leviathan::ini::INI_handler reader;
    reader.load(path);
    // reader.show();
    for (auto&& key : reader.get_sections())
    {
        std::cout << key << ',';
    }
    std::cout << "\n=================================" << std::endl;
    for (auto&& value : reader.get_entries())
    {
        std::cout << value << ',';
    }
    std::cout << "=================================" << std::endl;
    for (auto&& [k, v] : reader.get_items())
    {
        std::cout << "key is" << k << "and value is " << v << std::endl;
    }
    std::cout << "=================================" << std::endl;
}

void test_for_get()
{
    leviathan::ini::INI_handler reader;
    reader.load(path);
    console::write_lines_multi(
        *reader.getint("correct_test", "int"),
        *reader.getstring("correct_test", "src_path"),
        *reader.getfloat("correct_test", "float"),
        *reader.getboolean("correct_test", "switch"),
        *reader.getboolean("correct_test", "boolean"));
}

void test_for_write()
{
    leviathan::ini::INI_handler reader;
    // reader.load("D:/Library/Leviathan/result.txt");
    reader["correct_test"]["add1"] = "one";
    reader["correct_test"]["add2"] = "two";
    reader["correct_test"]["add3"];
    reader["new_section"]["add0"] = "zero";
    reader.show();
    reader.write("D:/Library/Leviathan/result.ini");
}