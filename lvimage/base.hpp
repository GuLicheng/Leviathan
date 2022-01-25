#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <compare>
#include <algorithm>
#include <bit>

#include <assert.h>
#include <stdint.h>


namespace leviathan::image 
{

struct image_buffer 
{
    int m_height;
    int m_width;
    int m_channels;
    std::vector<uint8_t> m_data;  
};

namespace tag
{
    struct BMP { };
};


constexpr uint32_t read_four_bytes(const uint8_t* p)  
{
    return p[0]     
        | (p[1] << 8)
        | (p[2] << 16)
        | (p[3] << 24);
}

constexpr uint16_t read_two_bytes(const uint8_t* p)
{
    return p[0] | (p[1] << 8);
}


template <typename OStream, typename... Ts>
void println(OStream& os, const Ts&... ts)
{
    (os << ... << ts) << std::endl;
}

}

