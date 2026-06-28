#include <leviathan/extc++/meta.hpp>
#include <leviathan/annotations/all.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <print>

struct A
{
    int AI;
};

struct B : A
{
    int BI;
};

struct C : A
{
    [[=cpp::refl::skip]]
    int CI;
};

struct D : B, C
{
    int DI;
};

struct [[=cpp::refl::uppercase]] MyStruct
{
    [[=cpp::refl::rename("RenamedField")]]
    int field1;

    int field2;

    struct [[=cpp::refl::selfname]] Inner
    {
        int field3;
    };

    enum class [[=cpp::refl::lowercase]] Color
    {
        Red [[=cpp::refl::rename("R")]],
        Green [[=cpp::refl::rename("G")]],
        Blue,
    };

};

int main(int argc, char const *argv[])
{
    template for (constexpr auto info : define_static_array(cpp::refl::all_nsdm_unchecked<std::tuple<int, double>>()))
    {
        std::print("{}\n", (display_string_of(info)));
    }

    auto s1 = cpp::refl::extract_name_by_annotation<^^MyStruct::field1>();
    auto s2 = cpp::refl::extract_name_by_annotation<^^MyStruct::field2>();
    auto s3 = cpp::refl::extract_name_by_annotation<^^MyStruct::Inner::field3>();
    auto s4 = cpp::refl::extract_name_by_annotation<^^MyStruct::Color::Red>();
    auto s5 = cpp::refl::extract_name_by_annotation<^^MyStruct::Color::Green>();
    auto s6 = cpp::refl::extract_name_by_annotation<^^MyStruct::Color::Blue>();

    std::println("{}", s1);
    std::println("{}", s2);
    std::println("{}", s3);
    std::println("{}", s4);
    std::println("{}", s5);
    std::println("{}", s6);

    return 0;
}
