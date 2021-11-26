#include <lv_cpp/config_parser/json/json.hpp>
#include <lv_cpp/utils/timer.hpp>
using namespace leviathan::json;

int main()
{
	system("chcp 65001");

	const char* json_path = R"(D:\Library\Leviathan\include\lv_cpp\config_parser\config\test.json)";
	std::fstream file{ json_path };
	std::string buf{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	json_reader j{ std::move(buf) };
	{
		leviathan::timer _;
		j.parse();
	}
	if (!j)
	{
		std::cerr << leviathan::json::report_error(j.error_code()) << '\n';
		return 1;
	}
	std::cin.get();
	auto& root = *(j.root().cast<json_object>());
	json_value number = json_number{ .m_val = 50 };
	number = json_boolean{ true };
	number.try_assign(json_boolean{ false });
	root.append(std::string("Hello"), json_string{ .m_val = "World" });
	std::cout << "============================================\n";
	j.serialize();

	std::cout << "===================================================\n";
	return 0;
}