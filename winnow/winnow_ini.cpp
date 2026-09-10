#include "all.hpp"
#include <print>

using Context = winnow::stream<winnow::context_error>;

class InIParser
{
public:

    static auto Parse(Context& ctx)
    {
        auto SectionLeftparenthesis = winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::literal("[").context(winnow::str_context(winnow::str_context_kind::expected, "Expected '['")),
            winnow::ascii::multispace0
        );

        auto SectionRightparenthesis = winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::literal("]"),
            winnow::ascii::multispace0
        );

        auto parser = winnow::combinator::delimited(
            SectionLeftparenthesis,
            winnow::ascii::alphanumeric1,
            SectionRightparenthesis
        );

        return parser(ctx);
    }

    static auto InICommit(Context& ctx)
    {
        return winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::literal(";"),
            winnow::ascii::till_line_ending
        )(ctx);
    }
};

template <typename Error, typename Need = size_t>
class ErrorMode
{
    union
    {
        Need need;
        Error error;
    };

    enum class State { Incomplete, Backtrack, Cut, Unknown } state;

    Error backtrack()
    {
        return this->error;
    }
};


int main()
{
    auto ctx = Context("[ section ]");

    auto result = InIParser::Parse(ctx);
    std::print("Result: {}\n", result.value());


    std::print("Hello, InIParser!\n");
}


