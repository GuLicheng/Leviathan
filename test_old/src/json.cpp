#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/utils/timer.hpp>
#include <algorithm>
using namespace leviathan::json;

int main()
{
	system("chcp 65001");

	const char* json_path = R"(D:\Library\Leviathan\include\leviathan\config_parser\config\test.json)";
	std::fstream file{ json_path };
	std::string buf{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	json_reader j{ std::move(buf) };
	{
		leviathan::timer _;
		j.parse();
		using namespace leviathan::json;
		auto& entries = j.root().cast<json_object>()->m_val;
		auto ls_ptr = std::ranges::find(entries, "performances", &json_entry::m_key);
			//.cast<json_array>();
		auto& ls = (ls_ptr->m_val.cast<json_array>()->m_val);
		int cnt = 0;
		for (auto& val : ls)
		{
			auto& obj = *val.cast<json_object>();
			auto& entries2 = obj.m_val;
			auto ls_ptr2 = std::ranges::find(entries2, "seatCategories", &json_entry::m_key)->m_val
				.cast<json_array>();
			auto& ls2 = ls_ptr2->m_val;
			for (auto& val2 : ls2)
			{
				auto& obj2 = *val2.cast<json_object>();
				auto& entries3 = obj2.m_val;
				auto ls_ptr3 = std::ranges::find(entries3, "areas", &json_entry::m_key)->m_val
					.cast<json_array>();
				auto& ls3 = ls_ptr3->m_val;
				for (auto& val3 : ls3)
				{
					auto& obj3 = *val3.cast<json_object>();
					auto& entries4 = obj3.m_val;
					cnt += std::ranges::count(entries4, "areaId", &json_entry::m_key);
				}
			}
		}
		std::cout << "num of areaId is :" << cnt << '\n'; // 8685
	}
	if (!j)
	{
		std::cerr << leviathan::json::report_error(j.error_code()) << '\n';
		return 1;
	}
	std::cin.get();
	std::cout << "============================================\n";
	j.serialize();
	std::cout << "===================================================\n";
	return 0;
}