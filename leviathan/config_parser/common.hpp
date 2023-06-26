#pragma once

#include "optional.hpp"

#include <fstream>
#include <string>
#include <cstdint>
#include <concepts>

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