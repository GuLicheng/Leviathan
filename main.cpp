#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <utility>
#include <mdspan>
#include <contracts>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <iostream>
#include <variant>
#include <print>
#include <format>
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



class A { };

class B : public A { };

class C : public B { };

class D : A, B, C { };

class TupleDerived1 : public cpp::tuple<double, int> { };
class TupleDerived2 : public std::tuple<double, int> { };

template <template <typename...> class Template, typename... Ts>
constexpr std::true_type match_base(const Template<Ts...>&);

template <template <typename...> class Template>
constexpr std::false_type match_base(...);

template <typename Derived, template <typename...> class Template>
struct is_derived_from_template
{
private:
    static constexpr decltype(auto) as_base(const Derived& d) noexcept {
        return static_cast<const Derived&>(d);
    }
public:
    static constexpr bool value = decltype(match_base<Template>(as_base(std::declval<Derived>())))::value;
};

template <typename Derived, template <typename...> class Template>
constexpr bool is_derived_from_template_v = is_derived_from_template<Derived, Template>::value;

// 全部替换为 X1 X2 X3 X4 X5
struct X1 : std::tuple<int> {};
struct X2 : std::tuple<float, double> {};
struct X3 : std::vector<int> {};
struct X4 {};
struct X5 : private std::tuple<int> {};

static_assert(is_derived_from_template_v<X1, std::tuple>);
static_assert(is_derived_from_template_v<X2, std::tuple>);
static_assert(!is_derived_from_template_v<X3, std::tuple>);
static_assert(!is_derived_from_template_v<X4, std::tuple>);
// static_assert(!is_derived_from_template_v<X5, std::tuple>);
static_assert(is_derived_from_template_v<X3, std::vector>);

template <typename D, template <typename...> class Tpl>
concept derived_from_template = is_derived_from_template_v<D, Tpl>;

int main()
{
    // template for (constexpr auto info : std::define_static_array(all_bases_unique<D>()))
    // {
    //     constexpr auto id = display_string_of(info);
    //     // constexpr auto id = display_string_of(type_of(info));
    //     std::print("Base: [{}]\n", id);
    // }

    // static_assert(is_derived_from_template<^^TupleDerived1, ^^cpp::tuple>());
    // static_assert(is_derived_from_template<^^TupleDerived2, ^^std::tuple>());
    // static_assert(!is_derived_from_template<^^D, ^^std::tuple>());

    return 0;
}




