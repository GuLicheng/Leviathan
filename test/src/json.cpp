#include <lv_cpp/config_parser/json/json.hpp>
using namespace leviathan::json;

int main()
{
    system("chcp 65001");
    try
    {
        const char* json_path = R"(D:\Library\Leviathan\include\lv_cpp\config_parser\json\test.json)";
        std::fstream file{ json_path };
        std::string buf{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        json_reader j{ std::move(buf) };
        j.parse();
        auto& root = *(j.root().cast<json_object>());
        json_value number = json_number{ .m_val = 50 }; 
        number = json_boolean{ true };
        number.try_assign(json_boolean{ false });
        root.append(std::string("Hello"), json_string{ .m_val = "World"});
        std::cout << "============================================\n";
        j.serialize();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    std::cout << "===================================================\n";
    return 0;
}