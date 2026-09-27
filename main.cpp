#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/core/context.hpp>
#include <leviathan/config_parser/core/parsers.hpp>
#include <print>
#include <meta>
#include <span>

class Context : public cpp::config::context
{
public:
    using cpp::config::context::context;
    using error_type = cpp::config::context_error;
};

int main()
{
    auto p = cpp::config::parser::sequence(
        cpp::config::parser::literal("Hello"),
        cpp::config::parser::literal("World"),
        cpp::config::parser::literal("!")
    );
    

    Context ctx("HelloWorld!");  

    auto r = ctx.apply(p);

    std::println("Result: {}", r.has_value());
    std::println("Result: {}", r.value());
    std::println("Result: {}", ctx.to_string_view());
}


