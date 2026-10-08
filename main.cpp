
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
#include <leviathan/config_parser/xml/xml.hpp>

class Context : public cpp::config::context
{
public:

    using cpp::config::context::context;
    using error_type = cpp::config::context_error;
};

int main()
{

}


