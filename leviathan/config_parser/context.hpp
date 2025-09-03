#pragma once

#include <string_view>

namespace cpp::config
{

template <typename CharT>
class basic_context
{
    std::basic_string_view<CharT> m_data;

public:

    using iterator = typename std::basic_string_view<CharT>::iterator;
    using const_iterator = typename std::basic_string_view<CharT>::const_iterator;
    using value_type = CharT;
    using size_type = typename std::basic_string_view<CharT>::size_type;
    using difference_type = typename std::basic_string_view<CharT>::difference_type;
    using reference = typename std::basic_string_view<CharT>::reference;
    using const_reference = typename std::basic_string_view<CharT>::const_reference;
    using reversed_iterator = typename std::basic_string_view<CharT>::reverse_iterator;
    using const_reversed_iterator = typename std::basic_string_view<CharT>::const_reverse_iterator;
    static constexpr size_type npos = std::basic_string_view<CharT>::npos;


    constexpr basic_context(std::basic_string_view<CharT> data) 
        : m_data(data)
    {
    }

    constexpr basic_context(const CharT* str)
        : m_data(str)
    {
    }

    constexpr const_iterator begin() const { return m_data.begin(); }
    constexpr iterator begin() { return m_data.begin(); }
    constexpr const_iterator cbegin() const { return m_data.cbegin(); }

    constexpr iterator end() { return m_data.end(); }
    constexpr const_iterator end() const { return m_data.end(); }
    constexpr const_iterator cend() const { return m_data.cend(); }

    constexpr size_type size() const { return m_data.size(); }
    constexpr bool empty() const { return m_data.empty(); }
    constexpr const_reference operator[](size_type pos) const { return m_data[pos]; }

    constexpr auto find_first_of(std::basic_string_view<CharT> sv, size_type pos = 0) const 
    { 
        return m_data.find_first_of(sv, pos); 
    }


    // Some other extended functionalities can be added if needed
    constexpr CharT peek(size_type offset) const
    {
        return offset < m_data.size() ? m_data[offset] : CharT(0);
    }

    constexpr CharT current() const { return peek(0); }
    constexpr CharT next() const { return peek(1); }
    
    // Unchecked advance
    constexpr void advance(size_type n) { m_data.remove_prefix(n); }
    
    constexpr bool try_advance(size_type n) 
    { 
        if (n > m_data.size())
        {
            return false;
        }
        advance(n); 
        return true; 
    }

    constexpr bool match(std::basic_string_view<CharT> str, bool consume) 
    {
        if (!m_data.starts_with(str))
        {
            return false;
        }

        if (consume)
        {
            advance(str.size());
        }
        return true;
    }

    constexpr bool match(CharT ch, bool consume) 
    {
        if (m_data.empty() || m_data[0] != ch)
        {
            return false;
        }

        if (consume)
        {
            advance(1);
        }
        return true;
    }

    constexpr std::basic_string_view<CharT> to_string_view() const { return m_data; }

    // Operators
    constexpr explicit operator bool() const { return !m_data.empty(); }


    // constexpr void remove_prefix(size_type n) { m_data.remove_prefix(n); }
    // constexpr void remove_suffix(size_type n) { m_data.remove_suffix(n); }


};
    
using context = basic_context<char>;
using wcontext = basic_context<wchar_t>;

} // namespace cpp::config

