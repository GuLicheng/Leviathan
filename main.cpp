#include <meta>
#include <format>
#include <string>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <leviathan/annotations/all.hpp>
#include <leviathan/extc++/format.hpp>
#include "nlohmann.hpp"

inline constexpr struct { } Fact;
inline constexpr struct { } Theory;
inline constexpr struct { } ParameterAnnotation;

struct [[=ParameterAnnotation, =cpp::derive::debug]] MyParameter
{
    [[=cpp::refl::rename("X")]]
    int x;
    double y;
    const char* z;
};

namespace N
{
    [[=Fact]]
    void fun1() { std::println("N::fun1 called"); }

    [[=Theory]]
    [[=MyParameter{.x = 1, .y = 3.14, .z = "hello"}]]
    void fun2(int x, double y, const char* z) 
    {
        std::println("N::fun2 called with x={}, y={}, z={}", x, y, z);
    }
};

// void show()
// {
//     template for (constexpr auto m : define_static_array(members_of(^^N, std::meta::access_context::current())))
//     {
//         if constexpr (cpp::refl::has_annotation(m, Fact)) 
//         {
//             std::invoke([:m:]);
//         }
//         else if constexpr (cpp::refl::has_annotation(m, Theory)) 
//         {
//             template for (constexpr auto a : define_static_array(annotations_of(m))) 
//             {
//                 if constexpr (cpp::refl::has_annotation(type_of(a), ParameterAnnotation)) 
//                 {
//                     constexpr auto [...params] = extract<typename [:type_of(a):]>(a);
//                     std::invoke([:m:], params...);
//                 }
//             }
//         }
//     }
// }

int main(int argc, char const *argv[]) 
{



    return 0;
}
