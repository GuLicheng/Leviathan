#pragma once

#include "common.hpp"

namespace cpp::config
{

class scanner
{
    std::string_view m_context;

    int m_line;

    static constexpr auto npos = std::string_view::npos;

public:

    explicit constexpr scanner(std::string_view context) : m_context(context), m_line(0) { }

    constexpr scanner(const scanner&) = delete;
    
    constexpr scanner& operator=(const scanner&) = delete;

    constexpr int line() const
    {
        return m_line;
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

    constexpr scanner slice(size_t pos, size_t count) const
    {
        // the substr will throw exception if pos > size().
        return scanner(m_context.substr(pos, count));
    }

    constexpr char peek(size_t n) const
    {
        return n >= m_context.size() ? 0 : m_context[n];
    }

    constexpr char current() const
    {
        return peek(0);
    }

    constexpr std::string_view read_line(bool remove) const
    {
        auto left = m_context.find('\n');
        auto right = left + 1;
        
        if (left == npos)
        {
            left = right = m_context.size();
        }

        ++m_line;
        
        auto retval = m_context.substr(0, left);

        if (remove)
        {
            m_context.remove_prefix(right);
        }
        
        return retval;
    }

    void advance_unchecked(size_t n)
    {
        auto prefix = m_context.substr(0, n);
        m_context.remove_prefix(n);
        m_line += std::ranges::count(prefix, '\n');
    }

    constexpr bool match_and_advance(char ch)
    {
        if (m_context.empty())
        {
            return false;
        }

        bool result = current() == ch;

        if (result)
        {
            if (ch == '\n')
            {
                ++m_line;
            }

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
        if (m_context.starts_with(context))
        {
            advance_unchecked(context.size());
            return true;
        }
        return false;
    }

    constexpr void advance(size_t n)
    {
        advance_unchecked(std::min(n, m_context.size()));
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

    constexpr void skip(std::string_view str)
    {
        auto idx = m_context.find_first_not_of(str);
        advance_unchecked(idx == npos ? size() : idx);
    }

    constexpr void skip_whitespace()
    {
        skip(" \r\n\t");
    }
    
    constexpr const char* data() const
    {
        return m_context.data();
    }

    constexpr bool is_newline() const
    {
        return m_context.starts_with('\n')
            || m_context.starts_with('\r')
            || m_context.starts_with("\r\n");  // The last case is not necessary.
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