#pragma once

namespace leviathan
{
    consteval bool is_utf8() 
    {
        constexpr unsigned char micro[] = "\u00B5";
        return sizeof(micro) == 3 && micro[0] == 0xC2 && micro[1] == 0xB5;
    }

    unsigned decode_unicode_from_char(const char* p)
    {
        auto to_digit = [](char ch) -> unsigned
        {
            if (ch >= '0' && ch <= '9') return ch - '0';
            if (ch >= 'a' && ch <= 'z') return ch - 'a' + 10;
            return ch - 'A' + 10;
        };
        unsigned i = 0;
        i |= to_digit(p[0]) << 12;
        i |= to_digit(p[1]) << 8;
        i |= to_digit(p[2]) << 4;
        i |= to_digit(p[3]);
        return i;
    }

    template <typename InputIterator>
    InputIterator encode_unicode_to_utf8(InputIterator iter, const unsigned codepoint)
    {
        if (codepoint <= 0x0000007F)
        {
            // * U-00000000 - U-0000007F:  0xxxxxxx
            *iter++ = (codepoint & 0x7F);
        }
        else if (codepoint >= 0x00000080 && codepoint <= 0x000007FF)
        {
            // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
            *iter++ = ((codepoint >> 6) & 0x1F) | 0xC0;
            *iter++ =  (codepoint & 0x3F)       | 0x80;
        }
        else if (codepoint >= 0x00000800 && codepoint <= 0x0000FFFF)
        {
            // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
            *iter++ = ((codepoint >> 12) & 0x0F) | 0xE0;
            *iter++ = ((codepoint >> 6) & 0x3F)  | 0x80;
            *iter++ =  (codepoint & 0x3F)        | 0x80;

        }
        else if (codepoint >= 0x00010000 && codepoint <= 0x001FFFFF)
        {
            // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            *iter++ = ((codepoint >> 18) & 0x07) | 0xF0;
            *iter++ = ((codepoint >> 12) & 0x3F) | 0x80;
            *iter++ = ((codepoint >> 6) & 0x3F)  | 0x80;
            *iter++ =  (codepoint & 0x3F)        | 0x80;
        }
        else if (codepoint >= 0x00200000 && codepoint <= 0x03FFFFFF)
        {
            // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            *iter++ = ((codepoint >> 24) & 0x03) | 0xF8;
            *iter++ = ((codepoint >> 18) & 0x3F) | 0x80;
            *iter++ = ((codepoint >> 12) & 0x3F) | 0x80;
            *iter++ = ((codepoint >> 6) & 0x3F)  | 0x80;
            *iter++ =  (codepoint & 0x3F)        | 0x80;
        }
        else if (codepoint >= 0x04000000 && codepoint <= 0x7FFFFFFF)
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
    /*
        system("chcp 65001");
        unsigned codepoints[] = {
            0x6211, 0x7231, 0x5317, 0x4eac, 0x5929, 0x5b89, 0x95e8
        }; 
        char buffer[128];
        char* pos = buffer;
        for (auto codepoint : codepoints)
            pos = leviathan::encode_unicode_to_utf8(pos, codepoint);
        std::cout << buffer << " - " << std::distance(buffer, pos); // 我爱北京天安门 - 21
    */

}