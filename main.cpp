#include <print>
#include <ranges>
#include <tuple>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/config_parser/json/json.hpp>

struct [[=cpp::derive::from<cpp::json::value>, =cpp::derive::debug]] Student
{
    std::string name;
    int age;
};

int main(int argc, char const *argv[])
{
    cpp::json::value v = {
        {"name", "Alice"},
        {"age", 30}
    };

    Student s = cpp::cast<Student>(v);

    std::println("Student name: {}, age: {}", s.name, s.age);

    return 0;
}



















