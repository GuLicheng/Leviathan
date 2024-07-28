#pragma once

#include <leviathan/string/string_extend.hpp>

#include <string>
#include <string_view>
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
inline std::string read_file_context(const char* filename, std::string_view newline = "\n")
{
    std::ifstream ifs(filename, std::ios_base::in | std::ios_base::binary);
    std::string context;
    
    if (ifs)
    {
        ifs.seekg(0, std::ios::end);
        context.resize(ifs.tellg());
        ifs.seekg(0, std::ios::beg);
        ifs.read(&context[0], context.size());
    }

    // return context;
    return leviathan::string::replace(std::move(context), newline, "\n");
}

inline void write_file(std::string_view content, const char* filename)
{
    std::ofstream ofs(filename, std::ios_base::out | std::ios_base::binary);

    if (ofs)
    {
        ofs.write(content.data(), content.size());
    }
}

} // namespace leviathan

