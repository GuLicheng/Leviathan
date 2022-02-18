#include <lv_cpp/enum.hpp>
#include <lv_cpp/named_tuple.hpp>

using person = named_tuple<
    field<"name", std::string>,
    field<"age", int>>;

int main()
{
    person p1 { "name"_arg = "Alice", "age"_arg = 10 };
    auto& name = p1["name"_arg];
    std::cout << name << '\n';
}

