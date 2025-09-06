#pragma once

#include <string_view>
#include <utility>
#include <assert.h>

namespace cpp::config
{

template <typename CharT>
struct parse_interface
{
    using iterator = typename std::basic_string_view<CharT>::iterator;
    using const_iterator = typename std::basic_string_view<CharT>::const_iterator;
    using value_type = CharT;
    using char_type = CharT;
    using size_type = typename std::basic_string_view<CharT>::size_type;
    using difference_type = typename std::basic_string_view<CharT>::difference_type;
    using reference = typename std::basic_string_view<CharT>::reference;
    using const_reference = typename std::basic_string_view<CharT>::const_reference;
    using reversed_iterator = typename std::basic_string_view<CharT>::reverse_iterator;
    using const_reversed_iterator = typename std::basic_string_view<CharT>::const_reverse_iterator;
    static constexpr size_type npos = std::basic_string_view<CharT>::npos;

    template <typename Self>
    constexpr std::basic_string_view<CharT> to_string_view(this Self& self)
    {
        return static_cast<std::basic_string_view<CharT>>(self);
    }

    template <typename Self>
    constexpr auto begin(this Self& self) 
    {   
        return self.to_string_view().begin();
    }

    template <typename Self>
    constexpr auto cbegin(this Self& self) 
    {   
        return self.to_string_view().cbegin();
    }

    template <typename Self>
    constexpr auto rbegin(this Self& self) 
    {   
        return self.to_string_view().rbegin();
    }

    template <typename Self>
    constexpr auto crbegin(this Self& self) 
    {   
        return self.to_string_view().crbegin();
    }

    template <typename Self>
    constexpr auto end(this Self& self) 
    {   
        return self.to_string_view().end();
    }

    template <typename Self>
    constexpr auto cend(this Self& self) 
    {   
        return self.to_string_view().cend();
    }

    template <typename Self>
    constexpr auto rend(this Self& self) 
    {   
        return self.to_string_view().rend();
    }

    template <typename Self>
    constexpr auto crend(this Self& self) 
    {   
        return self.to_string_view().crend();
    }
    
    template <typename Self>
    constexpr auto size(this const Self& self)
    {   
        return self.to_string_view().size();
    }

    template <typename Self>
    constexpr bool empty(this const Self& self)
    {   
        return self.to_string_view().empty();
    }

    template <typename Self>
    constexpr auto operator[](this const Self& self, size_type pos)
    {   
        return self.to_string_view()[pos];
    }

    template <typename Self>
    constexpr operator bool(this const Self& self) 
    {   
        return !self.to_string_view().empty();
    }

    template <typename Self>
    constexpr CharT peek(this const Self& self, size_type offset) 
    {   
        auto sv = self.to_string_view();
        return offset < sv.size() ? sv[offset] : CharT(0);
    }

    template <typename Self>
    constexpr CharT current(this const Self& self) 
    {   
        return self.peek(0);
    }

    template <typename Self>
    constexpr CharT next(this const Self& self)
    {   
        return self.peek(1);
    }

    template <typename Self>
    constexpr bool eof(this const Self& self) 
    {   
        return self.empty();
    }

    template <typename Self>
    constexpr void advance(this Self& self, size_type n) 
    {   
        self += n;
    }

    template <typename Self>
    constexpr bool try_advance(this Self& self, size_type n) 
    {   
        if (n > self.size())
        {
            return false;
        }
        self.advance(n); 
        return true; 
    }

    template <typename Self>
    constexpr bool match(this Self& self, std::basic_string_view<CharT> str, bool consume) 
    {   
        if (!self.to_string_view().starts_with(str))
        {
            return false;
        }

        if (consume)
        {
            self.advance(str.size());
        }
        return true;
    }

    template <typename Self>
    constexpr bool match(this Self& self, CharT ch, bool consume) 
    {   
        auto sv = self.to_string_view();
        if (sv.empty() || sv[0] != ch)
        {
            return false;
        }

        if (consume)
        {
            self.advance(1);
        }
        return true;
    }

    template <typename Self>
    constexpr std::pair<Self, Self> split_at(this const Self& self, size_type n) 
    {
        assert(n <= self.size());
        auto left = self, right = self;
        left -= (self.size() - n);
        right += n;
        return { left, right };
    }

    template <typename Self>
    constexpr auto find_first_of(this const Self& self, std::basic_string_view<CharT> sv, size_type pos = 0)
    { 
        return self.to_string_view().find_first_of(sv, pos); 
    }

    template <typename Self>
    constexpr auto skip_whitespace(this Self& self)
    {
        static auto whitespace = make_character_table([](size_t i)
        {
            [[assume(i < 256)]];
            constexpr std::string_view sv = " \r\n\t";
            return sv.contains(i);
        }); 

        for (; self.size() && whitespace[self[0]]; self.advance(1));
    }
};

template <typename CharT>
class basic_context : public parse_interface<CharT>
{
    std::basic_string_view<CharT> m_data;

public:

    using typename parse_interface<CharT>::size_type;

    constexpr basic_context(std::basic_string_view<CharT> data) 
        : m_data(data)
    {
    }

    constexpr basic_context(const CharT* str)
        : m_data(str)
    {
    }
    
    constexpr basic_context() = default;
    constexpr basic_context(const basic_context& other) = default;
    constexpr basic_context(basic_context&& other) noexcept = default;
    constexpr basic_context& operator=(const basic_context& other) = default;

    constexpr operator std::basic_string_view<CharT>() const { return m_data; }
    
    constexpr basic_context& operator+=(size_type n) 
    { 
        assert(n <= m_data.size());
        m_data.remove_prefix(n); 
        return *this; 
    }

    constexpr basic_context& operator-=(size_type n)
    {
        m_data.remove_suffix(n);
        return *this;
    }
};
    
using context = basic_context<char>;
using wcontext = basic_context<wchar_t>;

} // namespace cpp::config

