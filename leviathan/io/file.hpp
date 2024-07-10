#pragma once

#include <string>
#include <fstream>

namespace leviathan
{
    
/**
 * @brief: Read context from file.
 * 
 *  There are some comparisons:
 *  https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
 *  We chose the first technique which is the most faster way but may
 *  not brief.
 * 
 * @param filename Path for source
 * 
 * @return Empty string if file is cannot opened or empty file.
 */
inline std::string read_file_contents(const char* filename)
{
    std::fstream ifs(filename, std::ios_base::in | std::ios_base::binary);
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

} // namespace leviathan

