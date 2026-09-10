#include "all.hpp"
#include <print>

using Context = winnow::stream<winnow::context_error>;

class InIParser
{
public:

    static auto Parse(Context& ctx) {

        auto comment_consumer = winnow::combinator::delimited(
            winnow::combinator::preceded(
                winnow::ascii::multispace0,  // consume leading whitespace
                winnow::token::literal(";")  // comment indicator
            ),
            winnow::ascii::till_line_ending,
            winnow::ascii::line_ending
        );

        auto left_parenthesis = winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::literal("["),
            winnow::ascii::multispace0
        );

        auto right_parenthesis = winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::literal("]"),
            winnow::combinator::terminated(
                winnow::ascii::multispace0,
                winnow::combinator::opt(comment_consumer)
            )
        );

        auto identifier = winnow::token::take_while([](char c) {
            return std::isalnum(c) || c == '.';
        }, 1);

        auto section_parser = winnow::combinator::delimited(
            left_parenthesis,
            identifier,
            right_parenthesis
        );

        // key = value

        auto key_parser = winnow::combinator::delimited(
            winnow::ascii::multispace0,
            identifier,
            winnow::ascii::multispace0
        );

        auto value_parser = winnow::combinator::delimited(
            winnow::ascii::multispace0,
            winnow::token::take_while([](char c) {
                return ValidCharacters.contains(c);
            }, 1),
            winnow::combinator::terminated(
                winnow::ascii::multispace0,
                winnow::combinator::opt(comment_consumer)
            )
        );

        auto entry_parser = winnow::combinator::repeat(
            winnow::combinator::separated_pair(
                key_parser,
                winnow::token::literal("="),
                value_parser
            )
        );

        auto line_parser = winnow::combinator::repeat(
            winnow::combinator::sequence(
                section_parser,
                winnow::combinator::opt(entry_parser)
            )
        );

        auto parser = winnow::combinator::preceded(
            winnow::combinator::repeat()
        )
    }

    static constexpr std::string_view ValidCharacters = 
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789"
            "`~!@#$%^&*()-_=+[{]}\\|:'\",<.>/? ";

};


int main()
{
    auto ctx = Context("[ section ] ; This is commit");

    auto result = InIParser::Parse(ctx);
    std::print("Result: {}\n", result.value());


    std::print("Hello, InIParser!\n");
}


