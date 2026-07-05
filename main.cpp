#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/extc++/array.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <iostream>

template <typename T>
struct SkipAnnotation : cpp::refl::annotation
{
};

struct Rename1 : SkipAnnotation<int>
{
    constexpr std::string operator()(std::string old_name) 
    {
        return "new_" + old_name;
    }
};

struct Rename2 : cpp::refl::rename_annotation
{
    constexpr std::string operator()([[=Rename1()]] std::string old_name) 
    {
        return "another_" + old_name;
    }
};

struct MyStruct
{
    int a;
    [[=Rename1(), =Rename2(), =cpp::refl::skip]] double b;
    [[=cpp::refl::skip]] std::string c;
};

using TypeAlias = MyStruct;


int main(int argc, char const *argv[])
{
    constexpr static auto bases = define_static_array(cpp::refl::select_annotations_with_type(^^MyStruct::b, ^^cpp::refl::annotation));

    template for (constexpr auto base : bases)
    {
        std::print("base: [{}]\n", display_string_of(base));
    }

    TypeAlias c;

    std::cout << display_string_of(type_of(^^c)) << std::endl;

    static_assert(!std::meta::is_template(^^std::vector<int>));
    static_assert(std::meta::is_template(^^std::vector));

    return 0;
}



