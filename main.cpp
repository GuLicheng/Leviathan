
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

    void SimpleFunction(int thisValue, double thatValue)
    {
        this->advance(1);
    }
};

int main()
{
    auto sign = cpp::config::parser::alt(
        // cpp::config::parser::literal("+")
        cpp::config::parser::literal("-").value(-1)
        // cpp::config::parser::empty.value(1)
    );

    auto parser2 = cpp::config::parser::literal("123").map(
        [](std::string_view str) { return std::stoi(std::string(str)); }
    );

    Context ctx("8848");
    auto r = parser2(ctx);
    std::cout << show_name<decltype(r)>() << "\n";
}


