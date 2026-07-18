#include <leviathan/extc++/annotation.hpp>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <assert.h>


int main()
{
    template for (constexpr auto name : 
        define_static_array(cpp::refl::all_bases_of( ^^std::tuple<int, double> /* , std::meta::access_context::unchecked() */)))
    {
        std::println("{}", display_string_of(name));
    }

    std::println("{}", std::string("HelloWorld") | cpp::views::to_upper | std::ranges::to<std::string>());

}