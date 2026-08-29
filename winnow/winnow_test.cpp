
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

    auto parser2 = winnow::token::take_while([](char c) { return std::isalpha(c); }, 1);
    auto context2 = Context("abc123");
    auto result2 = parser2(context2);

    std::println("Result2: {}", result2.is_ok() ? "Ok" : "Err");
    std::println("Remaining2: {}", std::string_view(context2));
    
    auto parser3 = winnow::token::take_till([](char c) { return std::isdigit(c); }, 1);
    auto context3 = Context("abc123");
    auto result3 = parser3(context3);

    std::println("Result3: {}", result3.is_ok() ? "Ok" : "Err");
    std::println("Remaining3: {}", std::string_view(context3));

}