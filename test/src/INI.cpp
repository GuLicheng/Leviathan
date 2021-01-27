#include <lv_cpp/INI.hpp>
#include <functional>

constexpr const char* path = "./data.ini";

int main()
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
    std::cout << "Ok\n";
}