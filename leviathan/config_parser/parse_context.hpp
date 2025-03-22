#pragma once

#include "common.hpp"

namespace leviathan::config
{

struct parse_context
{
    static constexpr auto npos = std::string_view::npos;

    std::string_view m_ctx;

    constexpr parse_context(std::string_view context) : m_ctx(context) { }

    constexpr std::string_view read_line() const
    {
        auto left = m_ctx.find('\n');
        auto right = left + 1;
        
        if (left == m_ctx.npos)
        {
            left = right = m_ctx.size();
        }

        auto retval = m_ctx.substr(0, left);
        // m_ctx.remove_prefix(right);
        return retval;
    }

    constexpr char current() const
    {
        return peek(0);
    }

    constexpr bool match_and_advance(char ch)
    {
        if (m_ctx.empty())
        {
            return false;
        }

        bool result = current() == ch;

        if (result)
        {
            advance_unchecked(1);
        }
        return result;
    }

    constexpr bool consume(char ch)
    {
        return match_and_advance(ch);
    }

    constexpr bool consume(std::string_view context)
    {
        if (m_ctx.starts_with(context))
        {
            advance_unchecked(context.size());
            return true;
        }
        return false;
    }

    constexpr char peek(size_t n) const
    {
        return n >= m_ctx.size() ? 0 : m_ctx[n];
    }

    constexpr void advance(size_t n)
    {
        if (size() >= n)
        {
            advance_unchecked(n);
        }
        else
        {
            m_ctx = ""; // Reset context
        }
    }

    constexpr void advance_unchecked(size_t n)
    {
        m_ctx.remove_prefix(n);
    }

    constexpr bool match_literal_and_advance(std::string_view literal)
    {
        // compare, string_view::substr will check length automatically.
        if (m_ctx.compare(0, literal.size(), literal) != 0)
        {
            return false;
        }

        advance_unchecked(literal.size());
        return true;
    }

    constexpr bool match(char ch) const
    {
        return m_ctx.size() && m_ctx.front() == ch;
    }

    constexpr bool match(std::string_view prefix) const
    {
        return m_ctx.starts_with(prefix);
    }

    constexpr void skip(std::string_view str)
    {
        auto idx = m_ctx.find_first_not_of(str);
        m_ctx.remove_prefix(idx == m_ctx.npos ? m_ctx.size() : idx);
    }

    constexpr void skip_whitespace()
    {
        // auto idx = m_ctx.find_first_not_of(whitespace_delimiters);
        // m_ctx.remove_prefix(idx == m_ctx.npos ? m_ctx.size() : idx);
        struct whitespace_config
        {
            constexpr static int operator()(size_t i) 
            {
                [[assume(i < 256)]];
                constexpr std::string_view sv = " \r\n\t";
                return sv.contains(i);
            }
        };

        static auto whitespaces = make_character_table(whitespace_config());

        auto is_whitespace = [](char ch) { return whitespaces[ch]; };
        for (; m_ctx.size() && is_whitespace(current()); advance_unchecked(1));
    }

    constexpr bool is_at_end() const
    {
        return m_ctx.empty();
    }

    constexpr const char* data() const
    {
        return m_ctx.data();
    }

    constexpr size_t size() const
    {
        return m_ctx.size();
    }

    constexpr std::string_view slice(size_t offset = 0, size_t count = -1) const
    {
        offset = std::min(offset, m_ctx.size());
        // Exceptions: std::out_of_range if pos > size().
        return m_ctx.substr(offset, count);
    }

    constexpr bool is_newline() const
    {
        return m_ctx.starts_with('\n')
            || m_ctx.starts_with('\r')
            || m_ctx.starts_with("\r\n");  // The last case is not necessary.
    }

    constexpr bool eof() const
    {
        return is_at_end();
    }

    constexpr size_t search(std::string_view context)
    {
        return m_ctx.find(context);
    }

    constexpr void locate_character(char ch)
    {
        const auto idx = m_ctx.find(ch);
        advance_unchecked(idx == m_ctx.npos ? m_ctx.size() : idx);
    }

    // constexpr std::string_view locate(char ch) const
    // {
    //     auto other = *this;
    //     other.locate_character(ch);
    //     return { data(), data() + (size() - other.size()) };
    // }

    // Some operators
    constexpr char operator*() const
    {
        return current();
    }

    constexpr parse_context& operator++()
    {
        advance_unchecked(1);
        return *this;
    }

    constexpr explicit operator bool() const
    {
        return m_ctx.size();
    }

};

template <typename CharT>
class basic_parse_context
{
    using underlying_type = std::basic_string_view<CharT>;

    static constexpr auto npos = underlying_type::npos;

    underlying_type m_ctx;

public:

    constexpr basic_parse_context(underlying_type context) : m_ctx(context) { }

    constexpr CharT current() const
    {
        return peek(0);
    }

    constexpr bool match_and_advance(CharT ch)
    {
        if (m_ctx.empty())
        {
            return false;
        }

        bool result = current() == ch;

        if (result)
        {
            advance_unchecked(1);
        }
        return result;
    }

    constexpr bool consume(CharT ch)
    {
        return match_and_advance(ch);
    }

    constexpr bool consume(underlying_type context)
    {
        if (m_ctx.starts_with(context))
        {
            advance_unchecked(context.size());
            return true;
        }
        return false;
    }

    constexpr CharT peek(size_t n) const
    {
        return n >= m_ctx.size() ? CharT(0) : m_ctx[n];
    }

    constexpr void advance(size_t n)
    {
        if (size() >= n)
        {
            advance_unchecked(n);
        }
        else
        {
            // Reset context
            m_ctx = underlying_type(); 
        }
    }

    constexpr void advance_unchecked(size_t n)
    {
        m_ctx.remove_prefix(n);
    }

    constexpr bool match_literal_and_advance(underlying_type literal)
    {
        // compare, string_view::substr will check length automatically.
        if (m_ctx.compare(0, literal.size(), literal) != 0)
        {
            return false;
        }

        advance_unchecked(literal.size());
        return true;
    }

    constexpr bool match(CharT ch) const
    {
        return m_ctx.size() && m_ctx.front() == ch;
    }

    constexpr bool match(underlying_type prefix) const
    {
        return m_ctx.starts_with(prefix);
    }

    constexpr void skip(underlying_type str)
    {
        auto idx = m_ctx.find_first_not_of(str);
        m_ctx.remove_prefix(idx == m_ctx.npos ? m_ctx.size() : idx);
    }

    constexpr void skip_whitespace()
    {
        while (m_ctx.size() && 
            (m_ctx.front() == CharT(' ') || 
             m_ctx.front() == CharT('\t') || 
             m_ctx.front() == CharT('\n') || 
             m_ctx.front() == CharT('\r')))
        {
            advance_unchecked(1);
        }
    }

};

}  // leviathan::config


