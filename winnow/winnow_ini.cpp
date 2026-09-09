#include "all.hpp"
#include <print>

using Context = winnow::stream<winnow::context_error>;

class InIParser
{
    InIParser() = default;

    static auto SectionLeftparenthesis(Context& ctx)
    {
        return winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::literal("["),
            winnow::ascii::multispace0
        )(ctx);
    }

    static auto InICommit(Context& ctx)
    {
        return winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::literal(";"),
            winnow::ascii::till_line_ending
        )(ctx);
    }

    static auto SectionRightparenthesis(Context& ctx)
    {
        return winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::literal("]"),
            winnow::ascii::multispace0
        )(ctx);
    }
};


int main()
{
    std::print("Hello, InIParser!\n");
}


