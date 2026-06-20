#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <utility>
#include <mdspan>
#include <contracts>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <variant>
#include <print>
#include <format>
#include <tuple>
#include <leviathan/math/vector.hpp>

template <typename T>
struct type
{
    static constexpr void show_all_members()
    {
        constexpr static auto members = define_static_array(cpp::refl::all_nsdm_unchecked<T>());
        template for (constexpr auto member : members)
        {
            std::print("Member: {}\n", has_identifier(member) ? identifier_of(member) : "<unnamed>");
        }
    }
};

struct SomeInterafce
{
    template <typename Self>
    constexpr auto call_impl(this Self&& self)
    {
        auto f = self.[:member_named1<std::remove_cvref_t<Self>>("impl") :];
        return std::invoke(f, self);
    }
};

struct MyStruct {
    int X;
    double Y;
    int ReturnConstant() const { return 42; }
};

int main(int argc, char const argv[])
{
    MyStruct s;
    s.[:cpp::refl::member_named<MyStruct>("X") :] = 1;
    s.[:cpp::refl::member_named<MyStruct>("Y") :] = 3.14;
    assert(s.X == 1 && s.Y == 3.14);
    std::cout << (s.[:cpp::refl::member_named<MyStruct>("ReturnConstant") :]() == 42);
    return 0;
}




