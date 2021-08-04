#include <lv_cpp/collections/hash_table.hpp>
#include <lv_cpp/utils/struct.hpp>

int main()
{
	leviathan::hash_set<foo> h;
	h.insert(1);
	h.insert(2);
	h.insert(2);
	h.insert(3);
}