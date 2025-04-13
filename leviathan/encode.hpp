#pragma once

#include <cctype>
#include <cstdint>
#include <cstring>
#include <bit>
#include <string>
#include <string_view>

namespace cpp::encoding
{
    
constexpr bool is_valid_unicode(uint32_t codepoint)
{
    // basic multilingual plane, BMP(U+0000 - U+FFFF)
    return codepoint <= 0x10FFFF;
}   

template <typename CharT> struct UTF8;

template <typename CharT, std::endian Endian> struct UTF16;

template <typename CharT, std::endian Endian> struct UTF32;

// U+00000000 - U+0000007F: 0xxxxxxx
// U+00000080 - U+000007FF: 110xxxxx 10xxxxxx
// U+00000800 - U+0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
// U+00010000 - U+001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
// U+00200000 - U+03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
// U+04000000 - U+7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
template <>
struct UTF8<char>
{
    static constexpr int length(const char* start, int len)
    {
        int count = 0, i = 0;

        while (i < len)
        {
            const char cur = start[i];
            const auto ones = std::countl_one(static_cast<unsigned char>(cur));
            // We assume the bytes behind the first byte are valid.
            i += (ones == 0) ? 1 : ones;
            count++;
        }

        return count;
    }

    // We fix the second template parameter of std::basic_string_view to std::char_traits<char>
    static constexpr int length(std::basic_string_view<char> str)
    {
        return length(str.data(), str.size());
    }

    template <typename OutIterator>
    static constexpr OutIterator from_unicode(OutIterator iter, const uint32_t codepoint)
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
};

template <>
struct UTF16<char, std::endian::little>
{
    template <typename OutIterator>
    static constexpr OutIterator from_unicode(OutIterator iter, const uint32_t codepoint)
    {
        if (codepoint <= 0xFFFF)
        {
            *iter++ = (codepoint & 0xFF);
            *iter++ = (codepoint >> 8) & 0xFF;
        }
        else
        {
            // Surrogate pair
            // U+64321 = 0x50D9 0x21DF
            const uint32_t vx = codepoint - 0x10000;
            const uint32_t high = (vx >> 10) | 0xD800;
            const uint32_t low = (vx & 0x3FF) | 0xDC00;

            *iter++ = (high & 0xFF);
            *iter++ = (high >> 8) & 0xFF;
            *iter++ = (low & 0xFF);
            *iter++ = (low >> 8) & 0xFF;
        }
        return iter;
    }
};

template <>
struct UTF16<char, std::endian::big>
{
    template <typename OutIterator>
    static constexpr OutIterator from_unicode(OutIterator iter, const uint32_t codepoint)
    {
        if (codepoint <= 0xFFFF)
        {
            *iter++ = (codepoint >> 8) & 0xFF;
            *iter++ = (codepoint & 0xFF);
        }
        else
        {
            // Surrogate pair
            // U+64321 = 0xD950 0xDF21
            const uint32_t vx = codepoint - 0x10000;
            const uint32_t high = (vx >> 10) | 0xD800;
            const uint32_t low = (vx & 0x3FF) | 0xDC00;

            *iter++ = (high >> 8) & 0xFF;
            *iter++ = (high & 0xFF);
            *iter++ = (low >> 8) & 0xFF;
            *iter++ = (low & 0xFF);
        }
        return iter;
    }
};

} // namespace cpp::encoding

namespace cpp
{

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
template <typename OutIterator>
constexpr OutIterator encode_unicode_to_utf8(OutIterator iter, const uint32_t codepoint)
{
    return encoding::UTF8<char>::from_unicode(std::move(iter), codepoint);

    // if (codepoint <= 0x0000007F)
    // {
    //     // * U-00000000 - U-0000007F:  0xxxxxxx
    //     *iter++ = (codepoint & 0x7F);
    // }
    // else if (codepoint <= 0x000007FF)
    // {
    //     // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
    //     *iter++ = ((codepoint >> 6) & 0x1F) | 0xC0;
    //     *iter++ =  (codepoint & 0x3F)       | 0x80;
    // }
    // else if (codepoint <= 0x0000FFFF)
    // {
    //     // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
    //     *iter++ = ((codepoint >> 12) & 0x0F) | 0xE0;
    //     *iter++ = ((codepoint >> 6) & 0x3F)  | 0x80;
    //     *iter++ =  (codepoint & 0x3F)        | 0x80;
    // }
    // else if (codepoint <= 0x001FFFFF)
    // {
    //     // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    //     *iter++ = ((codepoint >> 18) & 0x07) | 0xF0;
    //     *iter++ = ((codepoint >> 12) & 0x3F) | 0x80;
    //     *iter++ = ((codepoint >> 6) & 0x3F)  | 0x80;
    //     *iter++ =  (codepoint & 0x3F)        | 0x80;
    // }
    // else if (codepoint <= 0x03FFFFFF)
    // {
    //     // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    //     *iter++ = ((codepoint >> 24) & 0x03) | 0xF8;
    //     *iter++ = ((codepoint >> 18) & 0x3F) | 0x80;
    //     *iter++ = ((codepoint >> 12) & 0x3F) | 0x80;
    //     *iter++ = ((codepoint >> 6) & 0x3F)  | 0x80;
    //     *iter++ =  (codepoint & 0x3F)        | 0x80;
    // }
    // else if (codepoint <= 0x7FFFFFFF)
    // {
    //     // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    //     *iter =   ((codepoint >> 30) & 0x01) | 0xFC;
    //     *iter++ = ((codepoint >> 24) & 0x3F) | 0x80;
    //     *iter++ = ((codepoint >> 18) & 0x3F) | 0x80;
    //     *iter++ = ((codepoint >> 12) & 0x3F) | 0x80;
    //     *iter++ = ((codepoint >> 6)  & 0x3F) | 0x80;
    //     *iter++ =  (codepoint & 0x3F)        | 0x80;
    // }
    // return iter;
}

} // namespace cpp

