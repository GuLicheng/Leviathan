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
        constexpr std::string_view name = has_identifier(member) ? identifier_of(member) : "<unnamed>";
        constexpr bool has_name = display_string_of(type_of(member)).contains("unnamed");
        std::print("class has name? {}, Type: {}, Member: {}\n", 
            has_name, display_string_of(type_of(member)), name);
    }
}

struct DebugInitializer
{
    int value = 0;

    void operator()(std::optional<int>& opt, std::string name)
    {
        // opt = value++;
    }
};

struct C
{
    struct [[=cpp::derive::debug]] { 
        int X; 
        int Y;
    };

    int Z;

    struct {
        int I1;
        int I2;
    } Inner;

    struct Bar {
        int B1;
        int B2;
    };
};

int main(int argc, char const* argv[])
{
    using Unnamed = decltype(C::Inner);

    std::optional<Unnamed> u;

    auto c = C(1, 2, 3, 4, 5);

    std::print("{}\n", c);
    return 0;
}
