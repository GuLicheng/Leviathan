#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <utility>
#include <mdspan>
#include <iostream>
#include <variant>
#include <print>
#include <format>
#include <leviathan/math/vector.hpp>
#include <tuple>

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

using Vector3f = cpp::math::vector<float, 3>;

int main(int argc, char const *argv[])
{

    Vector3f v1 = {1.0f, 2.0f, 3.0f};
    Vector3f v2 = {1.0f, 2.0f, 3.0f};
    Vector3f v3 = Vector3f::zero_vector;

    v1.equals(v2) ? std::print("v1 and v2 are approximately equal.\n") : std::print("v1 and v2 are not approximately equal.\n");

    auto r = v1 | v2;
    std::print("Dot product: {}\n", r);
    std::print("Euclidean distance: {}\n", Vector3f::euclidean_distance(v1, v3));
    std::print("Manhattan distance: {}\n", Vector3f::manhattan_distance(v1, v3));
    std::print("Chebyshev distance: {}\n", Vector3f::chebyshev_distance(v1, v3));
    std::print("v1 + v2: {}\n", v1 + v2);
    std::print("v1 * 2: {}\n", v1 * 2);

    return 0;
}

