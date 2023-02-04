#pragma once

#include <tuple>
#include <string>
#include <string_view>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <regex>

namespace lua
{

    enum class token_kind
    {
        TOKEN_EOF,                     // end of file
        TOKEN_VARARG,                  // ...

        TOKEN_SEP_SEMI,                // ;
        TOKEN_SEP_COMMA,               // ,
        TOKEN_SEP_DOT,                 // .
        TOKEN_SEP_COLON,               // :
        TOKEN_SEP_LABEL,               // ::
        TOKEN_SEP_LPAREN,              // (
        TOKEN_SEP_RPAREN,              // )
        TOKEN_SEP_LBRACK,              // [
        TOKEN_SEP_RBRACK,              // ]
        TOKEN_SEP_LCURLY,              // {
        TOKEN_SEP_RCURLY,              // }

        TOKEN_OP_ASSIGN,               // =
        TOKEN_OP_MINUS,                // -
        TOKEN_OP_WAVE,                 // ~
        TOKEN_OP_ADD,                  // +
        TOKEN_OP_MUL,                  // *
        TOKEN_OP_DIV,                  // /
        TOKEN_OP_IDIV,                 // //
        TOKEN_OP_POW,                  // ^
        TOKEN_OP_MOD,                  // %
        TOKEN_OP_BAND,                 // &
        TOKEN_OP_BOR,                  // - 
        TOKEN_OP_SHR,                  // >> 
        TOKEN_OP_SHL,                  // << 
        TOKEN_OP_CONCAT,               // .. 
        TOKEN_OP_LT,                   // < 
        TOKEN_OP_LE,                   // <= 
        TOKEN_OP_GT,                   // > 
        TOKEN_OP_GE,                   // >= 
        TOKEN_OP_EQ,                   // == 
        TOKEN_OP_NE,                   // ~= 
        TOKEN_OP_LEN,                  // # 
        TOKEN_OP_AND,                  // and 
        TOKEN_OP_OR,                   // or 
        TOKEN_OP_NOT,                  // not 

        // TOKEN_OP_UNM  = TOKEN_OP_MINUS  
        // TOKEN_OP_SUB  = TOKEN_OP_MINUS  
        // TOKEN_OP_BNOT = TOKEN_OP_WAVE
        // TOKEN_OP_BXOR = TOKEN_OP_WAVE


        TOKEN_KW_BREAK,                // break 
        TOKEN_KW_DO,                   // do 
        TOKEN_KW_ELSE,                 // else 
        TOKEN_KW_ELSEIF,               // elseif 
        TOKEN_KW_END,                  // end 
        TOKEN_KW_FALSE,                // false
        TOKEN_KW_FOR,                  // for
        TOKEN_KW_FUNCTION,             // function
        TOKEN_KW_GOTO,                 // goto
        TOKEN_KW_IF,                   // if
        TOKEN_KW_IN,                   // in
        TOKEN_KW_LOCAL,                // local
        TOKEN_KW_NIL,                  // nil
        TOKEN_KW_REPEAT,               // repeat
        TOKEN_KW_RETURN,               // return
        TOKEN_KW_THEN,                 // then
        TOKEN_KW_TRUE,                 // true
        TOKEN_KW_UNTIL,                // until
        TOKEN_KW_WHILE,                // while

        TOKEN_IDENTIFIER,              // identifier
        TOKEN_NUMBER,                  // number literal
        TOKEN_STRING,                  // string literal
        SENTINEL,
    };

    inline std::unordered_map<std::string_view, token_kind> keywords = {
        { "and",      token_kind::TOKEN_OP_AND },
        { "break",    token_kind::TOKEN_KW_BREAK },
        { "do",       token_kind::TOKEN_KW_DO },
        { "else",     token_kind::TOKEN_KW_ELSE },
        { "elseif",   token_kind::TOKEN_KW_ELSEIF },
        { "end",      token_kind::TOKEN_KW_END },
        { "false",    token_kind::TOKEN_KW_FALSE },
        { "for",      token_kind::TOKEN_KW_FOR },
        { "function", token_kind::TOKEN_KW_FUNCTION },
        { "goto",     token_kind::TOKEN_KW_GOTO },
        { "if",       token_kind::TOKEN_KW_IF },
        { "in",       token_kind::TOKEN_KW_IN },
        { "local",    token_kind::TOKEN_KW_LOCAL },
        { "nil",      token_kind::TOKEN_KW_NIL },
        { "not",      token_kind::TOKEN_OP_NOT },
        { "or",       token_kind::TOKEN_OP_OR },
        { "repeat",   token_kind::TOKEN_KW_REPEAT },
        { "return",   token_kind::TOKEN_KW_RETURN },
        { "then",     token_kind::TOKEN_KW_THEN },
        { "true",     token_kind::TOKEN_KW_TRUE },
        { "until",    token_kind::TOKEN_KW_UNTIL },
        { "while",    token_kind::TOKEN_KW_WHILE },
    };

    class lexer
    {
        std::string m_chunk;
        std::string m_chunk_name;
        int m_line;

        inline static auto re_opening_long_bracket = R"(^\[=*\[)";

        bool test(const std::string& msg) 
        {
            return msg.starts_with(msg);
        }

        void skip_comment() 
        {
            next(2); // skip '--'
            if (test("["))
            {
                // long comment ?
                struct LongCommonNotImplError { };
                throw LongCommonNotImplError();
            }

            // short comment
            for (; m_chunk.size() && !is_new_line(m_chunk.front());)
                next(1);
        }

        void next(int n) 
        {
            m_chunk = m_chunk.substr(n);    
        }

        bool is_new_line(char ch) 
        {
            return ch == 'r' || ch == '\n';
        }

        bool is_white_space(char ch) 
        {
            static const char space_characters[] = { '\t', '\n', '\v', '\f', '\r', ' ' };
            return std::ranges::find(space_characters, ch) != std::ranges::end(space_characters);
        }

        void skip_white_space() 
        {
            // std::string_view chunk = m_chunk;
            while (m_chunk.size() > 0)
            {
                if (test("--"))
                {
                    skip_comment();
                }
                else if (test("\r\n") || test("\n\r"))
                {
                    next(2);
                    m_line++;
                }
                else if (is_new_line(m_chunk.front())) 
                {
                    next(1);
                    m_line++;
                }
                else if (is_white_space(m_chunk.front()))
                {
                    next(1);
                }
                else
                {
                    break;
                }
            }
        }

    public:

        lexer(std::string chunk, std::string chunk_name)
            : m_chunk(chunk), m_chunk_name(m_chunk_name), m_line(1) { }

        std::tuple<int, token_kind, std::string> next_token() 
        {
            skip_white_space();
            if (m_chunk.empty())
                return { m_line, token_kind::TOKEN_EOF, "EOF" };
        }


    };


    void show_keywords()
    {
        for (auto [k, v] : lua::keywords)
            std::cout << k << ": " << (int)v << '\n'; 
    }


}





