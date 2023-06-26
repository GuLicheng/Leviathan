#pragma once

#include "optional.hpp"

#include <fstream>
#include <string>
#include <cstdint>

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
    inline optional<int64_t> string_to_integer(const std::string& str, int base = 10)
    {
        int& err = errno;
        const char* ptr = str.c_str();
        char* end_ptr;
        err = 0;

        const auto ans = ::strtoll(ptr, &end_ptr, base);

        if (ptr == end_ptr || err == ERANGE)
        {
            return nullopt;
        }
        return ans;
    }

    inline optional<double> string_to_floating(const std::string& str)
    {
        int& err = errno;
        const char* ptr = str.c_str();
        char* end_ptr;
        err = 0;

        const auto ans = ::strtod(ptr, &end_ptr);

        if (ptr == end_ptr || err == ERANGE)
        {
            return nullopt;
        }
        return ans;
    }
}