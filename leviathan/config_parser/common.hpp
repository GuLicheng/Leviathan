#pragma once

#include <leviathan/config_parser/optional.hpp>

#include <array>
#include <fstream>
#include <string>
#include <cstdint>
#include <concepts>

#include <ctype.h>


namespace leviathan::config
{
    inline std::string read_file_contents(const char* filename)
    {
        std::fstream ifs(filename, std::ios_base::in | std::ios_base::binary);
        
        // Slow. https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
        // return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()); 

        std::string contents;

        if (ifs)
        {
            ifs.seekg(0, std::ios::end);
            contents.resize(ifs.tellg());
            ifs.seekg(0, std::ios::beg);
            ifs.read(&contents[0], contents.size());
        }
        return contents;
    }
}

namespace leviathan::config
{
    template <std::integral I>
    constexpr optional<I> from_chars_to_optional(const char* startptr, const char* endptr, int base = 10)
    {
        I value;
        auto result = std::from_chars(startptr, endptr, value, base);
        if (result.ec == std::errc() && result.ptr == endptr)
            return value;
        return nullopt;
    }

    template <std::floating_point F>
    constexpr optional<F> from_chars_to_optional(const char* startptr, const char* endptr, std::chars_format fmt = std::chars_format::general)
    {
        F value;
        auto result = std::from_chars(startptr, endptr, value, fmt);
        if (result.ec == std::errc() && result.ptr == endptr)
            return value;
        return nullopt;
    }
}

namespace leviathan::config
{
    inline constexpr std::array<int, 256> digit_values = []() {

        std::array<int, 256> values;

        values.fill(-1);

        for (int i = 0; i < 10; ++i) values['0' + i] = i;        // 0 - 9
        for (int i = 0; i < 26; ++i) values['a' + i] = i + 10;   // a - z
        for (int i = 0; i < 26; ++i) values['A' + i] = i + 10;   // A - z

        return values;
    }();

    consteval bool is_utf8() 
    {
        constexpr unsigned char micro[] = "\u00B5";
        return sizeof(micro) == 3 && micro[0] == 0xC2 && micro[1] == 0xB5;
    }

    /**
     * @brief Decode unicode from C-style string. 
     * 
     * @param p The string to be decoded, please make sure that the p has at least 4 bytes.
    */
    constexpr unsigned decode_unicode_from_char(const char* p)
    {
        auto to_digit = [](char ch) -> unsigned
        {
            return digit_values[ch];
            // if (ch >= '0' && ch <= '9') return ch - '0';
            // if (ch >= 'a' && ch <= 'z') return ch - 'a' + 10;
            // return ch - 'A' + 10;
        };

        unsigned i = 0;
        i |= to_digit(p[0]) << 12;
        i |= to_digit(p[1]) << 8;
        i |= to_digit(p[2]) << 4;
        i |= to_digit(p[3]);
        return i;
    }

    /**
     * @brief Encode codepoint to char-array.
     * 
     * @return Position after encoding codepoint, if the codepoint is invalid, the
     *  iter will not increased.
     * 
     * E.g.
     *   system("chcp 65001");
     *   unsigned codepoints[] = { 0x6211, 0x7231, 0x5317, 0x4eac, 0x5929, 0x5b89, 0x95e8 }; 
     *   char buffer[128] = {};
     *   char* pos = buffer;
     *   for (auto codepoint : codepoints)
     *      pos = encode_unicode_to_utf8(pos, codepoint);
     *   std::cout << buffer << " - " << std::distance(buffer, pos); // 我爱北京天安门 - 21
    */
    template <typename InputIterator>
    constexpr InputIterator encode_unicode_to_utf8(InputIterator iter, const unsigned codepoint)
    {
        if (codepoint <= 0x0000007F)
        {
            // * U-00000000 - U-0000007F:  0xxxxxxx
            *iter++ = (codepoint & 0x7F);
        }
        else if (codepoint <= 0x000007FF)
        {
            // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
            *iter++ = ((codepoint >> 6) & 0x1F) | 0xC0;
            *iter++ =  (codepoint & 0x3F)       | 0x80;
        }
        else if (codepoint <= 0x0000FFFF)
        {
            // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
            *iter++ = ((codepoint >> 12) & 0x0F) | 0xE0;
            *iter++ = ((codepoint >> 6) & 0x3F)  | 0x80;
            *iter++ =  (codepoint & 0x3F)        | 0x80;
        }
        else if (codepoint <= 0x001FFFFF)
        {
            // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            *iter++ = ((codepoint >> 18) & 0x07) | 0xF0;
            *iter++ = ((codepoint >> 12) & 0x3F) | 0x80;
            *iter++ = ((codepoint >> 6) & 0x3F)  | 0x80;
            *iter++ =  (codepoint & 0x3F)        | 0x80;
        }
        else if (codepoint <= 0x03FFFFFF)
        {
            // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            *iter++ = ((codepoint >> 24) & 0x03) | 0xF8;
            *iter++ = ((codepoint >> 18) & 0x3F) | 0x80;
            *iter++ = ((codepoint >> 12) & 0x3F) | 0x80;
            *iter++ = ((codepoint >> 6) & 0x3F)  | 0x80;
            *iter++ =  (codepoint & 0x3F)        | 0x80;
        }
        else if (codepoint <= 0x7FFFFFFF)
        {
            // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            *iter =   ((codepoint >> 30) & 0x01) | 0xFC;
            *iter++ = ((codepoint >> 24) & 0x3F) | 0x80;
            *iter++ = ((codepoint >> 18) & 0x3F) | 0x80;
            *iter++ = ((codepoint >> 12) & 0x3F) | 0x80;
            *iter++ = ((codepoint >> 6)  & 0x3F) | 0x80;
            *iter++ =  (codepoint & 0x3F)        | 0x80;
        }
        return iter;
    }
}