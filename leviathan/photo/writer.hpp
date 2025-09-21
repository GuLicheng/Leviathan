#pragma once

#include "bmp.hpp"

#include <string_view>

namespace cpp::image
{

void write(const char* filename, const buffer& buf)
{
    std::string_view fname(filename);

    if (fname.ends_with(".bmp"))
    {
        bmp::writer::write(filename, buf);
    }
    else
    {
        throw std::runtime_error("Unsupported image format");
    }
}

}

