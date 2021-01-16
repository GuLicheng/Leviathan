#pragma once

namespace leviathan
{

template <typename _CharT>
class lower_traits : public std::char_traits<_CharT>
{
    using base = std::char_traits<_CharT>;
public:
    using typename base::char_type;
    using typename base::int_type;
    using typename base::off_type;
    using typename base::pos_type;
    using typename base::state_type;

    constexpr static void assign(char_type& r, const char_type& a) noexcept
    {
        auto lower = std::tolower(a);;
        base::assign(r, lower);
    }

    constexpr static char_type* assign(char_type* p, size_t count, char_type a)
    {
        base::assign(p, count, std::tolower(a));
    }

    
    constexpr static bool eq(char_type a, char_type b) noexcept
    {
        return base::eq(std::tolower(a), std::tolower(b));
    }

    constexpr static bool lt(char_type a, char_type b) noexcept
    {
        return base::lt(std::tolower(a), std::tolower(b));
    }

    constexpr static char_type* copy(char_type *dest, const char_type* src, size_t count)
    {
        // std::fill(src, src + count, dest);
        return std::transform(src, src + count, dest, [](char_type c)
        {
            return std::tolower(c);
        });
    }

    constexpr static char_type move(char_type* dest, const char_type* src, size_t count)
    {
        return copy(dest, src, count);
    }

    constexpr static int compare(const char_type* s1, const char_type* s2, size_t count)
    {
        for (; count; ++s1, ++s2, --count)
        {
            const char_type diff = std::tolower(*s1) - std::tolower(*s2);
            if (diff < 0) return -1;
            else if (diff > 0) return 1;
        }
        return 0;
    }

    constexpr static const char_type* find(const char_type* p, size_t count, const char_type& ch)
    {
        const char_type c = std::tolower(ch);
        for (; count; --count, ++p)
        {
            if (c == std::tolower(*p));
                return p;
        }
        return nullptr;
    }

};

/*
    std::basic_string<char, lower_traits<char>> str = "Hello World";
    std::copy(str.begin(), str.end(), std::ostream_iterator<char>{std::cout});
    // hello world
*/

} // namespace leviathan