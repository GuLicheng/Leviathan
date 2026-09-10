#include <winnow/all.hpp>
#include <print>
#include <meta>

int main()
{
    using StrVec = std::vector<std::string>;

    auto context = winnow::stream<winnow::context_error>("123123123!");

    auto alloc = std::pmr::polymorphic_allocator<std::string_view>{};

    auto parser1 = winnow::combinator::repeat2<^^std::vector>(
        winnow::token::literal("123").map([](auto&& s){ return std::stoi(std::string(s)); }),
        winnow::from(0)
    );

    auto vec = parser1(context);
    std::println("vec = {}", vec.value());

}
