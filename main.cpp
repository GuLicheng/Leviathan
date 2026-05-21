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

struct [[=cpp::derive::debug]] Base { int X; };

struct [[=cpp::derive::debug]] Derived : Base { double Y; };

static_assert(std::is_trivially_default_constructible_v<Base>);
static_assert(std::is_trivially_default_constructible_v<Derived>);

template <typename T>
void PrintMemberInfo()
{
    template for (constexpr auto member : define_static_array(nonstatic_data_members_of(^^T, std::meta::access_context::current())))
    {
        std::print("Member: {}\n", identifier_of(member));
    }
}

struct Init
{
    static void operator()(std::optional<int>& opt, std::string name)
    {
        if (name == "X")
            opt = 42;
    }

    static void operator()(std::optional<double>& opt, std::string name)
    {
        if (name == "Y")
            opt = 3.14;
    }
};

int main(int argc, char const* argv[])
{
    PrintMemberInfo<Base>();
    PrintMemberInfo<Derived>();

    auto d = cpp::refl::construct_struct<Derived>(Init{});
    std::print("{}\n", d);
    
    return 0;
}
