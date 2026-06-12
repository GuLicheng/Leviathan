#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <utility>
#include <mdspan>
#include <iostream>
#include <variant>
#include <print>
#include <tuple>

template <typename T>
void ShowName()
{
    std::cout << display_string_of(^^T) << std::endl;
}


/*
    Undefined<0> { std::variant<std::monostate> value; };

    consteval { Something<int>(); }

    Undefined<1> { std::variant<std::monostate, int> value; };

    ListAppend<std::variant<std::monostate>, int> -> std::variant<std::monostate, int>

*/

template <typename List, typename T> struct ListAppend;

template <template <typename...> typename Template, typename... Ts, typename T>
struct ListAppend<Template<Ts...>, T> {
    using type = std::conditional_t<(((std::same_as<Ts, T>) || ...)), Template<Ts...>, Template<Ts..., T>>;
};

template <size_t N> struct Undefined;

// Class ^^Undefined
template <std::meta::info Class>
struct VariantBuilder {

    static consteval bool is_defined(size_t index) {
        return is_complete_type(substitute(Class, {std::meta::reflect_constant(index)}));
    }

    static consteval size_t get_last_index() {
        size_t k = 0;
        for (; is_defined(k); ++k);
        return k;
    }

    template <typename T>   
    static consteval void put()
    {
        constexpr auto index = get_last_index();
        define_aggregate(
            substitute(Class, { std::meta::reflect_constant(index) }),
            { std::meta::data_member_spec(^^T, {.name = "value"}) }
        );
    }

    template <size_t Index = get_last_index() - 1>
    using get_t = decltype(Undefined<Index>::value);

    template <typename T>
    static consteval void update_variant()
    {
        using CurrentType = get_t<>;
        using NextType = typename ListAppend<CurrentType, T>::type;
        put<NextType>();
    }

};

using Builder = VariantBuilder<^^Undefined>;

consteval {
    Builder::put<std::variant<std::monostate>>(); 
    Builder::update_variant<double>();
    Builder::update_variant<bool>();
}

struct Foo {
    int a;
    double b;
};

enum class Color { Red, Green, Blue };

consteval {
    Builder::update_variant<Foo>();
    Builder::update_variant<Color>();
}


int main(int argc, char const *argv[])
{
    using T = Builder::get_t<>;
    ShowName<T>();

    return 0;
}

