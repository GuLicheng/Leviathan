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

consteval std::vector<std::meta::info> all_bases_of(std::meta::info info)
{
    auto bases = std::views::concat(std::vector{std::meta::dealias(info)}, bases_of(info, std::meta::access_context::unchecked()) 
            | std::views::transform(std::meta::type_of)
            | std::views::transform(all_bases_of)
            | std::views::join)
            | std::ranges::to<std::vector>();

    std::vector<std::meta::info> result;
    std::ranges::copy_if(bases, std::back_inserter(result), [&](auto info) {
        return !std::ranges::contains(result, info, std::meta::dealias);
    }, std::meta::dealias);
    return result;
}

consteval bool is_derived_from(std::meta::info derived, std::meta::info base)
{
    return std::ranges::contains(all_bases_of(derived), dealias(base), std::meta::dealias);
}

consteval bool is_derived_from_template(std::meta::info derived, std::meta::info template_info)
{
}

int main(int argc, char const *argv[])
{
    constexpr static auto bases = define_static_array(all_bases_of(^^Bar));

    template for (constexpr auto base : bases)
    {
        std::print("base: [{}]\n", display_string_of(base));
    }

    static_assert(is_derived_from(^^Bar, ^^Base1));

    return 0;
}



