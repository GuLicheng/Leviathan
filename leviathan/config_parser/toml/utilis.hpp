#pragma once

#include "../common.hpp"
#include "../parse_context.hpp"
#include "value.hpp"

#include <regex>
#include <ranges>
#include <cmath>

namespace leviathan::config::toml::detail
{

// We define some constant values here.
inline constexpr std::string_view wschar = " \t";
inline constexpr std::string_view ml_literal_string_delim = "'''";
inline constexpr std::string_view ml_basic_string_delim = "\"\"\"";
inline constexpr std::string_view array_table_open = "[[";
inline constexpr std::string_view array_table_close = "]]";

// For convenience, we keep linefeed be '\n' during reading file.
inline constexpr char newline = '\n';
inline constexpr char quotation_mark = '"';
inline constexpr char apostrophe = '\'';
inline constexpr char dot_sep = '.';
inline constexpr char escaped = '\\';
inline constexpr char keyval_sep = '=';
inline constexpr char comment_start_symbol = '#';
inline constexpr char std_table_open = '[';
inline constexpr char std_table_close = ']';
inline constexpr char array_open = '[';
inline constexpr char array_close = ']';
inline constexpr char array_sep = ',';
inline constexpr char inline_table_open = '{';
inline constexpr char inline_table_close = '}';
inline constexpr char inline_table_sep = ',';

template <size_t N, typename InputIterator>
void decode_unicode(InputIterator dest, parse_context& ctx) 
{
    if (ctx.size() != N)
    {
        throw_toml_parse_error("Too small characters for unicode");
    }

    auto codepoint = decode_unicode_from_char<N>(ctx.data());
    encode_unicode_to_utf8(dest, codepoint);
    ctx.advance_unchecked(N);
}

template <typename Fn>
string parse_basic_string_impl(parse_context& ctx, Fn fn)
{
    ctx.consume('\"');
    std::string out;

    while (ctx)
    {
        if (*ctx == detail::escaped)
        {
            ++ctx; // eat '\\'
            check_and_throw(!ctx.eof(), "Escaped cannot at the end.");

            switch (*ctx)
            {
                case '"': out += '"'; ++ctx; break;    // quote
                case '\\': out += '\\'; ++ctx; break;  // backslash
                case 'b': out += '\b'; ++ctx; break;   // backspace
                case 'f': out += '\f'; ++ctx; break;   // formfeed
                case 'n': out += '\n'; ++ctx; break;   // linefeed
                case 'r': out += '\r'; ++ctx; break;   // carriage return
                case 't': out += '\t'; ++ctx; break;   // horizontal tab
                case 'u': decode_unicode<4>(std::back_inserter(out), ++ctx); break;
                case 'U': decode_unicode<8>(std::back_inserter(out), ++ctx); break;
                // TODO: \x -> two digits
                default: throw_toml_parse_error("Illegal character after escaped.");
            }
        }
        else if (*ctx == '\"') 
        {
            return out;
        }
        else
        {
            check_and_throw(fn(*ctx), "Invalid character {}.", *ctx);
            out += *ctx;
            ++ctx;
        }
    }

    throw_toml_parse_error("Basic string should end with \".");
}

inline std::optional<string> remove_underscore(std::string_view sv)
{
    if (sv.front() == '_' || sv.back() == '_' || sv.contains("__"))
    {
        return nullopt;
    }
    return sv 
            | std::views::filter([](char ch) { return ch != '_'; }) 
            | std::ranges::to<std::string>();
}

inline std::optional<integer> parse_integer(std::string_view sv)
{
    auto s = remove_underscore(sv);

    if (!s)
    {
        return nullopt;
    }

    sv = s.value();  // Replace sv with s which is removed underscore.

    if (sv.front() == '+') // from_chars cannot parse '+'
    {
        sv.remove_prefix(1);
    }

    int base = 10;

    if (sv.size() > 1 && sv.front() == '0')
    {
        switch (sv[1])
        {
            case 'x':
            case 'X': base = 16; break;
            case 'o':
            case 'O': base = 8; break;
            case 'b':
            case 'B': base = 2; break;
            default: return nullopt;  
        }
        sv.remove_prefix(2);
    }

    if (auto op = from_chars_to_optional<integer>(sv, base); op)
    {
        return *op;
    }
    return nullopt;
}

inline std::optional<floating> parse_float(std::string_view sv)
{
    auto s = remove_underscore(sv);

    if (!s)
    {
        return nullopt;
    }

    sv = s.value();  // Replace sv with s which is removed underscore.

    // .7 or 7. is not permitted.
    if (sv.front() == '.' || sv.back() == '.')
    {
        return nullopt;
    }

    floating sign = [ch = sv.front()]() {
        switch (ch) 
        {
            case '+': return 1;
            case '-': return -1;
            default: return 0;
        }
    }();

    if (sign == 0)
    {
        // "nan" and "inf" is valid for std::from_chars
        if (auto op = from_chars_to_optional<floating>(sv); op) 
        {
            return *op;
        }
        return nullopt;
    }

    sv.remove_prefix(1); // eat '+' or '-'

    if (sv.front() == '.')
    {
        // +.7 is also not permitted.
        return nullopt;
    }

    if (auto op = from_chars_to_optional<floating>(sv); op) 
    {
        // std::copysign is valid for nan/inf
        return std::copysign(*op, sign);
    }
    return nullopt;
}

inline std::optional<datetime> as_option_if(bool ok, const datetime& dt)
{
    if (ok)
    {
        return dt;
    }
    else
    {
        return std::nullopt;
    }
}

inline std::optional<datetime> parse_time(std::string_view sv)
{
    datetime dt;
    parse_context ctx(sv);

    // HH:MM:SS.PRECISION OFFSET
    if (ctx.peek(2) == ':')
    {
        // HH:MM:SS or HH:MM:SS.PRECISION
        auto hh = from_chars_to_optional<uint8_t>(ctx.slice(0, 2));
        auto mm = from_chars_to_optional<uint8_t>(ctx.slice(3, 2));

        if (hh && mm)
        {
            dt.m_time.m_hour = hh.value();
            dt.m_time.m_minute = mm.value();
        }
        else
        {
            return std::nullopt;
        }

        // https://github.com/toml-lang/toml/blob/main/toml.abnf
        // partial-time   = time-hour ":" time-minute [ ":" time-second [ time-secfrac ] ]
        ctx.advance_unchecked(5);

        if (ctx.match(':'))
        {
            auto ss = from_chars_to_optional<uint8_t>(ctx.slice(1, 2));

            if (ss)
            {
                dt.m_time.m_second = ss.value();
            }
            else
            {
                return std::nullopt;
            }

            ctx.advance_unchecked(3);
        }

        // Precision
        if (ctx.match('.'))
        {
            ctx.advance_unchecked(1); // eat '.'
            size_t i = 0;
            for (; ::isdigit(ctx.peek(i)); ++i);
            auto nano = from_chars_to_optional<uint32_t>(ctx.slice(0, i));
            
            if (nano)
            {
                dt.m_time.m_nanosecond = nano.value();
                ctx.advance_unchecked(i);
                return dt;
            }
            else
            {
                return std::nullopt;
            }
        }
        else if (ctx.match('Z') || ctx.match('z'))
        {
            // z must be last character of context.
            dt.m_offset.m_minute = 0;
            return as_option_if(ctx.size() == 1, dt);
        }
        else if (ctx.match('+') || ctx.match('-'))
        {
            int sign = ctx.match('+') ? 1 : -1;
            ctx.advance_unchecked(1);

            // -07:00 -> HH:SS
            auto hhh = from_chars_to_optional<int>(ctx.slice(0, 2));
            auto sss = from_chars_to_optional<int>(ctx.slice(3, 2));

            if (hhh && sss)
            {
                dt.m_offset.m_minute = (hhh.value() * 60 + sss.value()) * sign;
            }
            else
            {
                return std::nullopt;
            }

            // The offset should be last part.
            return as_option_if(ctx.size() == 5, dt);
        }
        else 
        {
            return as_option_if(ctx.eof(), dt);
        }
    }
    else
    {
        return std::nullopt;
    }
}

inline std::optional<datetime> parse_datetime(std::string_view sv)
{
    parse_context ctx(sv);

    // 1987-07-05T17:45:00Z
    // 1979-05-27T00:32:00.999999-07:00
    // 1979-05-27T00:32:00-07:00
    auto dtopt = parse_time(sv);

    if (!dtopt)
    {
        if (ctx.peek(4) != '-' || ctx.peek(7) != '-')
        {
            return std::nullopt;
        }

        datetime dt;
        auto year = from_chars_to_optional<uint16_t>(ctx.slice(0, 4));
        auto month = from_chars_to_optional<uint8_t>(ctx.slice(5, 2));
        auto day = from_chars_to_optional<uint8_t>(ctx.slice(8, 2));
        
        if (year && month && day)
        {
            dt.m_data.m_year = year.value();
            dt.m_data.m_month = month.value();
            dt.m_data.m_day = day.value();
        }
        else
        {
            return std::nullopt;
        }

        ctx.advance_unchecked(10);
 
        if (ctx.match('T') || ctx.match('t') || ctx.match(' '))
        {
            ctx.advance_unchecked(1);
            dtopt = parse_time(ctx.slice());

            if (!dtopt)
            {
                return std::nullopt;
            }
            else
            {
                dt.m_time = dtopt->m_time;
                dt.m_offset = dtopt->m_offset;
                return dt;
            }
        }
        else
        {
            return as_option_if(ctx.eof(), dt);
        }
    }
    else
    {
        return dtopt;
    }
}

} // namespace leviathan::config::toml::detail

