#include <leviathan/annotations/all.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/meta.hpp>
#include <print>
#include <variant>
#include <leviathan/config_parser/json/json.hpp>

struct
[[=cpp::derive::debug]]
[[=cpp::derive::decode<cpp::json::value>]]
[[=cpp::derive::encode<cpp::json::value>]]
Foo
{
    [[=cpp::refl::skip]]
    int x = 1;
    std::string y = "hello";
    std::vector<double> z = { 1.0, 2.0, 3.0 };
};

template<typename T>
consteval std::meta::info make_integer_seq_refl(T N) {
    std::vector args{ ^^T };
    for (T k = 0; k < N; ++k) {
        args.push_back(std::meta::reflect_constant(k));
    }
    return substitute(^^std::integer_sequence, args);
}




int main(int argc, char const* argv[])
{
    auto t = cpp::refl::struct_to_tuple<Foo>({ .x = 42, .y = "world", .z = { 4.0, 5.0 } });
    std::print("{}", t);
    return 0;
}
