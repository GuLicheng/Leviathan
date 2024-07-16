#pragma once

#include <ctype.h>

namespace leviathan
{
    
consteval bool is_utf8() 
{
    constexpr unsigned char micro[] = "\u00B5";
    return sizeof(micro) == 3 && micro[0] == 0xC2 && micro[1] == 0xB5;
}

/**
 * @brief Check whether code is valid(each character should 
 *  be xdigit.
 * 
 * @param p Unicode sequence to be encoded, please make sure that the p has at least N bytes.
 * @param N Length of unicode, 4 for \u and 8 for \U.
*/
template <size_t N>   
constexpr bool is_unicode(const char* p)
{
    static_assert(N == 4 || N == 8);

    if constexpr (N == 4)
    {
        return isxdigit(p[0])
            && isxdigit(p[1])
            && isxdigit(p[2])
            && isxdigit(p[3]);
    }
    else
    {
        return isxdigit(p[0])
            && isxdigit(p[1])
            && isxdigit(p[2])
            && isxdigit(p[3])
            && isxdigit(p[4])
            && isxdigit(p[5])
            && isxdigit(p[6])
            && isxdigit(p[7]);
    }
}

/**
 * @brief Encode codepoint to char-array.
 * 
 * @return Position after encoding codepoint, if the codepoint is invalid, the
 *  iter will not increase.
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
constexpr InputIterator encode_unicode_to_utf8(InputIterator iter, const uint32_t codepoint)
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

} // namespace leviathan

