
#include <optional>
#include <print>
#include <meta>
#include <iostream>
#include <span>

template <typename T>
consteval std::string_view show_name()
{
    return display_string_of(^^T);
}

#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/core/context.hpp>
#include <leviathan/config_parser/core/parsers.hpp>

class Context : public cpp::config::context
{
public:
    using cpp::config::context::context;
    using error_type = cpp::config::context_error;
};

int main()
{
    auto ctx = Context("Hello World!");

    using ErrorType = cpp::config::err_mode<std::string>;

    auto parser2 = cpp::config::parser::digit1.map_err([](auto&& err) 
    {
        return 0;
    });

    auto result = ctx.apply(parser2);

    std::cout << "Result type: " << show_name<decltype(result)>() << std::endl;

}


