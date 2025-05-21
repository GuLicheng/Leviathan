#pragma once

#include "common.hpp"

namespace cpp::config
{

class scanner
{
    static constexpr auto npos = std::string_view::npos;

    std::string_view m_context;

    int m_line;

    int m_column; 

public:

    explicit constexpr scanner(std::string_view context) : m_context(context), m_line(0), m_column(0) { }

    constexpr scanner(const scanner&) = delete;
    
    constexpr scanner& operator=(const scanner&) = delete;

    // Observers
    constexpr int line() const
    {
        return m_line;
    }

    constexpr int column() const
    {
        return m_column;
    }

    constexpr std::string_view context() const
    {
        return m_context;
    }

    constexpr size_t size() const
    {
        return m_context.size();
    }

    constexpr size_t empty() const
    {
        return m_context.empty();
    }

    constexpr bool match(char ch) const
    {
        return current() == ch;
    }

    constexpr bool match(std::string_view prefix) const
    {
        return m_context.starts_with(prefix);
    }

    constexpr bool eof() const
    {
        return m_context.empty();
    }
    
    constexpr const char* data() const
    {
        return m_context.data();
    }
    
    // constexpr bool is_newline() const
    // {
    //     return m_context.starts_with('\n')
    //         || m_context.starts_with('\r')
    //         || m_context.starts_with("\r\n");  // The last case is not necessary.
    // }

    // constexpr scanner slice(size_t pos, size_t count) const
    // {
    //     // the substr will throw exception if pos > size().
    //     return scanner(m_context.substr(pos, count));
    // }

    constexpr char peek(size_t n) const
    {
        return n >= m_context.size() ? 0 : m_context[n];
    }

    constexpr char current() const
    {
        return peek(0);
    }

    constexpr std::string_view read_line() const
    {
        auto left = m_context.find('\n');
        auto right = left + 1;
        
        if (left == npos)
        {
            left = right = size();
        }

        return m_context.substr(0, left);
    }

    void advance_unchecked(size_t n)
    {
        // auto prefix = m_context.substr(0, n);
        // m_context.remove_prefix(n);
        // m_line += std::ranges::count(prefix, '\n');

        for (size_t i = 0; i < n; ++i)
        {
            if (prefix[i] == '\n')
            {
                m_column = 0;
                ++m_line;
            }
            else
            {
                ++m_column;
            }
        }
    }

    constexpr bool match_and_advance(char ch)
    {
        return consume(ch);
    }

    constexpr bool consume(char ch)
    {
        if (current() == ch)
        {
            advance_unchecked(1);
            return true;
        }
        return false;
    }
    
    constexpr bool consume(std::string_view context)
    {
        if (m_context.starts_with(context))
        {
            advance_unchecked(context.size());
            return true;
        }
        return false;
    }

    constexpr void advance(size_t n)
    {
        advance_unchecked(std::min(n, size()));
    }

    constexpr void skip(std::string_view str)
    {
        auto idx = m_context.find_first_not_of(str);
        advance_unchecked(idx == npos ? size() : idx);
    }

    constexpr void skip_whitespace()
    {
        skip(" \r\n\t");
    }

    // The comment should be started with comment, and the end of comment is '\n'.
    constexpr void skip_whitespace_and_comment(std::string_view comment)
    {
        assert(match(comment));
        consume(comment);
        locate_character('\n');
    }

    constexpr void locate_character(char ch)
    {
        const auto idx = m_context.find(ch);
        advance_unchecked(idx == npos ? size() : idx);
    }

    // Some operators
    constexpr char operator*() const
    {
        return current();
    }

    explicit constexpr operator bool() const
    {
        return !eof();
    }

    constexpr scanner& operator++()
    {
        advance_unchecked(1);
        return *this;
    }

    constexpr scanner& operator+=(size_t n)
    {
        advance_unchecked(n);
        return *this;
    }
};

}  // namespace cpp::config