
#include "all.hpp"
#include <print>

using Context = winnow::stream<winnow::context_error>;

int main()
{
    auto parser = winnow::token::literal("hello");
    auto context = Context("hello world");
    auto result = parser(context);

    std::println("Result: {}", result.is_ok() ? "Ok" : "Err");
    std::println("Remaining: {}", std::string_view(context));
}