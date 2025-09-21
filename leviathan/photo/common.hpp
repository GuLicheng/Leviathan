#pragma once

#include <bit>
#include <fstream>

namespace cpp::image
{
    
inline uint32_t read_four_bytes(const uint8_t* p, std::endian endian = std::endian::native)  
{
    const uint32_t result = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
    return endian == std::endian::little ? result : std::byteswap(result);
}

inline uint16_t read_two_bytes(const uint8_t* p, std::endian endian = std::endian::native)
{
    const uint16_t result = p[0] | (p[1] << 8);
    return endian == std::endian::little ? result : std::byteswap(result);
}

inline uint16_t read_u16(std::ifstream& fs, std::endian endian = std::endian::native)
{
    uint8_t buffer[2];
    fs.read(reinterpret_cast<char*>(buffer), 2);
    return read_two_bytes(buffer, endian);
}

inline uint32_t read_u32(std::istream& fs, std::endian endian = std::endian::native)
{
    uint8_t buffer[4];
    fs.read(reinterpret_cast<char*>(buffer), 4);
    return read_four_bytes(buffer, endian);
}

inline uint8_t read_u8(std::istream& fs)
{
    uint8_t buffer[1];
    fs.read(reinterpret_cast<char*>(buffer), 1);
    return buffer[0];
}

// template <typename T>
// void write_bytes(std::ostream& os, const T* valptr, int size = sizeof(T), std::endian endian = std::endian::native)
// {
//     // fs.write(reinterpret_cast<const char*>(padding_bytes), pad)
//     os.write("HelloWorld", 10);
//     os.write(reinterpret_cast<const char*>(valptr), size);
//     os.write("HelloWorld", 10);
// }

} // namespace cpp::image

