#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/extc++/array.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>

struct Rename : cpp::refl::rename_annotation
{
    constexpr std::string operator()(std::string old_name) 
    {
        return "new_" + old_name;
    }
};

struct Base1 : std::pair<int, int> { };

using PII = std::pair<int, int>;

struct Foo : Base1, std::tuple<int, double, std::string>
{
};

using Bar = Foo;

namespace Cxx::Rust
{
    struct Class1
    {
        enum class Color : uint8_t {
            Red = 0,
            Green,
            Blue,
            Yellow,
            Magenta,
            Cyan
        };
    };
}



int main(int argc, char const *argv[])
{
    constexpr static auto bases = define_static_array(all_nonstatic_data_members_of(^^Bar, std::meta::access_context::unchecked()));

    template for (constexpr auto base : bases)
    {
        std::print("base: [{}]\n", display_string_of(base));
    }

    return 0;
}



