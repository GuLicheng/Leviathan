#pragma once

#include <leviathan/extc++/string.hpp>

#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <leviathan/extc++/ranges.hpp>

namespace cpp
{
 
inline constexpr struct 
{
    /**
     * @brief: C++ version of python.os.listdir
     * 
     * @return std::vector<std::string>
     */
    static auto operator()(const char* path, bool fullname = false)
    {
        auto nameof = [=](const auto& entry) {
            return fullname ? entry.path().string() : entry.path().filename().string();
        };

        return std::filesystem::directory_iterator{path}
             | cpp::views::transform(nameof)
             | std::ranges::to<std::vector>();
    }

} listdir;

inline constexpr struct 
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
     * @param newline Manner of interpreting text streams in which 
     *  all of the following are recognized as ending a line: the Unix 
     *  convention, '\n', the Windows convention, '\r\n', and the old Macintosh 
     *  convention, '\r'.
     * 
     * @return file content as a string.
     * 
     * @throw std::runtime_error if file is not found.
     */
    static std::string operator()(const char* filename, std::string_view newline = "\n")
    {
        std::ifstream ifs(std::string(filename), std::ios_base::in | std::ios_base::binary);
        std::string context;
        
        if (ifs)
        {
            ifs.seekg(0, std::ios::end);
            context.resize(ifs.tellg());
            ifs.seekg(0, std::ios::beg);
            ifs.read(&context[0], context.size());
        }
        else
        {
            throw std::runtime_error("File not found: " + std::string(filename));
        }

        // return context;
        return cpp::string::replace(std::move(context), newline, "\n");
    }

    static auto operator()(const std::string& path)
    {
        return operator()(path.c_str());
    }

} read_file_context;

inline constexpr struct
{
    /**
     * @brief: Write context to file.
     * 
     * @param context Context to write
     * @param filename Path for destination
     * 
     * @return True if file is written successfully, otherwise false.
     */
    static bool operator()(std::string_view context, const char* filename)
    {
        std::ofstream ofs(filename, std::ios_base::out | std::ios_base::binary);

        if (ofs)
        {
            ofs.write(context.data(), context.size());
            return true;
        }

        return false;
    }
} write_file;

} // namespace cpp

