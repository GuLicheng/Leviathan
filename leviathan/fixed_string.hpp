// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0259r0.pdf
#pragma once

#include <cstddef>
#include <algorithm>
#include <compare>      // std::string_ordering
#include <functional>   // std::hash
#include <format>       // std::formatter
#include <iterator>
#include <array>

namespace cpp::string
{

template <size_t N, typename CharT, typename Traits = std::char_traits<CharT>>
struct basic_fixed_string
{
    using value_type = CharT;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = value_type*;
    using const_iterator = const value_type*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;
    using string_view_type = std::basic_string_view<value_type, Traits>;

    static constexpr auto npos = string_view_type::npos;

    constexpr size_t size() const noexcept 
    { return N; }
    
    constexpr size_t length() const noexcept 
    { return N; }
    
    constexpr bool empty() const noexcept 
    { return size() == 0; }

    constexpr size_t capacity() const noexcept
    { return N; }

    constexpr size_t max_size() const noexcept
    { return N; }

    constexpr basic_fixed_string() = default;

    constexpr basic_fixed_string(const basic_fixed_string& str) noexcept
    {
        Traits::copy(m_data, str.m_data, N + 1);
    }

    constexpr basic_fixed_string(const CharT (&str)[N + 1]) noexcept
    {
        // static constexpr char_type* copy( char_type* dest, const char_type* src, std::size_t count );
        Traits::copy(m_data, str, N + 1);
    }

    constexpr basic_fixed_string& operator=(const basic_fixed_string& str) noexcept
    {
        Traits::copy(m_data, str.m_data, N + 1);
    }

    constexpr basic_fixed_string& operator=(const CharT (&str)[N + 1]) noexcept
    {
        Traits::copy(m_data, str, N + 1);
    }

    constexpr string_view_type sv() const noexcept
    { return { begin(), end() }; }

    constexpr explicit operator string_view_type() const noexcept
    { return sv(); }

    template <size_t K>
    constexpr auto operator<=>(const basic_fixed_string<K, CharT, Traits>& rhs) const noexcept
    { return sv() <=> rhs.sv(); }

    // Support comparison for strings with different length.
    template <size_t K>
    constexpr bool operator==(const basic_fixed_string<K, CharT, Traits>& rhs) const noexcept
    { 
        if constexpr (N == K)
            return sv() == rhs.sv();
        else
            return false;
    }

    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const basic_fixed_string& rhs) 
    {
        return os.write(rhs.data(), N);
    }

    constexpr value_type* data() noexcept { return m_data; }
    constexpr const value_type* data() const noexcept { return m_data; }

    constexpr const value_type* c_str() const noexcept { return m_data; }

    constexpr value_type& operator[](size_type pos) noexcept { return m_data[pos]; }
    constexpr const value_type& operator[](size_type pos) const noexcept { return m_data[pos]; }

    constexpr reference front() noexcept { return *begin(); }
    constexpr const_reference front() const noexcept { return *begin(); }

    constexpr reference back() noexcept { return *(end() - 1); }
    constexpr const_reference back() const noexcept { return *(end() - 1); }

    constexpr value_type& at(size_type pos) 
    { 
        if (pos >= size()) 
            throw std::out_of_range("index out of boundary"); 
        return m_data[pos]; 
    }    

    constexpr const value_type& at(size_type pos) const 
    { 
        if (pos >= size()) 
            throw std::out_of_range("index out of boundary"); 
        return m_data[pos]; 
    }    

    constexpr iterator begin() noexcept { return m_data; }
    constexpr iterator end() noexcept { return m_data + N; }
    
    constexpr const_iterator begin() const noexcept { return m_data; }
    constexpr const_iterator end() const noexcept { return m_data + N; }

    constexpr const_iterator cbegin() const noexcept { return m_data; }
    constexpr const_iterator cend() const noexcept { return m_data + N; }
    
    constexpr reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }
    constexpr reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }

    constexpr const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end()); }
    constexpr const_reverse_iterator rend() const noexcept { return std::make_reverse_iterator(begin()); }

    constexpr const_reverse_iterator rcbegin() const noexcept { return std::make_reverse_iterator(end()); }
    constexpr const_reverse_iterator rcend() const noexcept { return std::make_reverse_iterator(begin()); }

    constexpr std::size_t hash_code() const noexcept { return std::hash<string_view_type>()(sv()); }

    CharT m_data[N + 1]; // literal operator requires m_data must be public now.
};

template <size_t N, typename CharT>
basic_fixed_string(const CharT (&str)[N]) -> basic_fixed_string<N - 1, CharT>; 
// char[N] is different from const char* with n characters

// Early GCC versions that support cNTTP were not able to deduce size_t parameter
// of basic_fixed_string when fixed_string and other typedef were just type aliases.
template <size_t N> using fixed_string = basic_fixed_string<N, char>;
template <size_t N> using fixed_wstring = basic_fixed_string<N, wchar_t>;


} // namespace cpp

// Implement std::hash and std::formatter for basic_fixed_string.
template <size_t N, typename CharT, typename Traits>
struct std::hash<cpp::string::basic_fixed_string<N, CharT, Traits>>
{
    constexpr size_t 
    operator()(const ::cpp::string::basic_fixed_string<N, CharT, Traits>& x) const  
    { return x.hash_code(); }
};

template <size_t N, typename CharT, typename Traits>
struct std::formatter<cpp::string::basic_fixed_string<N, CharT, Traits>, CharT>
    : std::formatter<basic_string_view<CharT, Traits>>
{
    using self = std::formatter<cpp::string::basic_fixed_string<N, CharT, Traits>, CharT>;

    constexpr formatter() = default;

    using base = formatter<basic_string_view<CharT, Traits>>;

    constexpr typename basic_format_parse_context<CharT>::iterator
    parse(std::basic_format_parse_context<CharT>& context)
    { return base::parse(context); }

    template <typename Out>
    typename basic_format_context<Out, CharT>::iterator
    format(const self& fs, std::basic_format_context<Out, CharT>& fc) const
    { return base::format(fs.sv(), fc); }
};

namespace cpp::literals
{
    // std::cout << "{}-{}-{:5d}"_fs("Hello", "World", 2);
    template <string::basic_fixed_string FixedString> 
    constexpr auto operator ""_fs() 
    {
        return []<typename... Args>(Args&&... args) static {
            return std::vformat(FixedString.sv(), std::make_format_args((Args&&) args...));
        };
    }
}

namespace cpp
{

using string::basic_fixed_string;
using string::fixed_string;
using string::fixed_wstring;

}


