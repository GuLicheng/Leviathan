#pragma once

#include <vector>

namespace cpp::image
{
    
struct buffer
{
    int height;
    int width;
    int channels;
    std::vector<uint8_t> data;
};

} // namespace cpp::image

