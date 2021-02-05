#include <lv_cpp/INI.hpp>
#include <iostream>

constexpr const char* path = R"(D:\Library\Leviathan\data.ini)";

void test_for_read();
void test_for_get();

int main()
{
    test_for_get();
    std::cout << "Ok\n";
}

void test_for_read()
{
    // std::function<void(int, int)> a = std::swap<int, int>;
    leviathan::INI::INI_handler reader;
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
    leviathan::INI::INI_handler reader;
    reader.load(path);
    std::cout << *reader.getint("section_one", "height") << std::endl;
    std::cout << *reader.getfloat("section_one", "width") << std::endl;
}