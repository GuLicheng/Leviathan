#include <meta>
#include <initializer_list> 
#include <span>
#include <print>
#include <leviathan/annotations/all.hpp>
#include <leviathan/extc++/meta.hpp>
#include <list>

struct [[=cpp::refl::value_annotation]] VectorString
{
    static std::vector<std::string> operator()() {
        return {"hello", "world"};
    }
};

struct F
{
    int Age; // 200 -> json: { "Age": 200 }

    [[=cpp::refl::default_array({1, 2, 3})]]
    std::list<int> ints;

    [[=VectorString()]]
    std::vector<std::string> strings;
};

int main(int argc, char const *argv[])
{
    auto f = cpp::refl::construct_struct<F>([](auto&&...) { });
    std::println("ints: {}", f.ints);
    std::println("strings: {}", f.strings);

    return 0;
}
