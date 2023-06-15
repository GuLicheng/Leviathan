#pragma once

#include <fstream>
#include <string>

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
