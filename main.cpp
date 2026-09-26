#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/core/context.hpp>
#include <print>
#include <meta>
#include <span>

enum class TokenType
{
    LeftBracket,
    RightBracket,
    Identifier,
};

struct Token
{
    TokenType type;
    std::string_view value;
};

struct TokenStream : public cpp::config::token_interface<Token>
{
    std::span<Token> tokens;
};

int main()
{
    std::vector<Token> tokens = {
        { TokenType::LeftBracket, "[" },
        { TokenType::Identifier, "content" },
        { TokenType::RightBracket, "]" }
    };

    std::span<Token> tokenSpan = tokens;

    using T = typename TokenStream::underlying_type;

    // tokenSpan.data();
    // tokenSpan.
}


