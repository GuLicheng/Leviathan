/*
    https://www.w3.org/TR/png-3/
*/

#pragma once

#include "common.hpp"
#include "buffer.hpp"

#include <utility>
#include <fstream>
#include <vector>
#include <span>
#include <algorithm>
#include <string_view>

#include <assert.h>

namespace cpp::image::png
{
    
inline bool check_png_signature(const unsigned char* signature)
{
    const unsigned char png_signature[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    return std::equal(signature, signature + 8, png_signature);
}

consteval uint32_t chunk_type(std::string_view sv)
{
    assert(sv.size() == 4);
    // static_assert(sv.size() == 4, "String view must be exactly 4 characters long");
    return (static_cast<uint32_t>(sv[0]) << 24) |
           (static_cast<uint32_t>(sv[1]) << 16) |
           (static_cast<uint32_t>(sv[2]) << 8)  |
           (static_cast<uint32_t>(sv[3]));
}

template <typename T>
struct lz77_token
{
    int offset;       // Distance to the start of the match in the search buffer
    int length;       // Length of the match
    T next_char;      // The next character after the match
};

/**
 * @brief LZ77 compression algorithm implementation
 * 
 * @param bytes Input byte stream to be compressed
 * @param window_size Size of the sliding window (search buffer)
 * @param buffer_size Size of the look-ahead buffer
 * 
 * @return Compressed byte stream
 */
template <typename T>
inline std::vector<lz77_token<T>> lz77_compress(const std::span<T> bytes, int window_size, int buffer_size)
{
    assert((size_t)window_size < 256 && (size_t)buffer_size < 256);
    static_assert(sizeof(T) == 1, "Only support byte stream");

    int left = 0;   // Left boundary of the sliding window
    int right = 0;  // Right boundary of the look-ahead buffer
    std::vector<lz77_token<T>> tokens;

    while (right < bytes.size())
    {
        auto window = bytes.subspan(left, right - left);  // Search buffer
        auto buffer = bytes.subspan(right, std::min(buffer_size, static_cast<int>(bytes.size() - right))); // Look-ahead buffer
        int offset = 0;

        // Find the longest match in the Look-ahead buffer
        while (buffer.size())
        {
            auto result = std::ranges::search(window, buffer);

            if (result != window.end())
            {
                offset = static_cast<int>(result - window.begin());
                break;
            }
            else
            {
                buffer = buffer.subspan(0, buffer.size() - 1); // Reduce the look-ahead buffer size
            }
        }

        // if (buffer.empty())
        // {
        //     // No match found
        //     tokens.push_back({ 0, 0, bytes[right] });
        //     right += 1;
        // }
        // else
        // {
        //     // Match found
        //     int offset = static_cast<int>(window.end() - result);
        //     int length = static_cast<int>(buffer.size());
        //     T next_char = (right + length < bytes.size()) ? bytes[right + length] : 0; // Handle end of stream

        //     tokens.push_back({ offset, length, next_char });
        //     right += length + 1;
        // }
    }

    return tokens;
}

// https://zhuanlan.zhihu.com/p/673817002
inline buffer read_png_file(const char* filename)
{
    std::ifstream ifs(filename, std::ios::binary | std::ios::in);

    if (!ifs.is_open())
    {
        throw std::runtime_error("Failed to open PNG file");
    }

    // Read and validate PNG signature -> "89 50 4E 47 0D 0A 1A 0A"
    unsigned char signature[8];
    ifs.read(reinterpret_cast<char*>(signature), 8);

    if (!check_png_signature(signature))
    {
        throw std::runtime_error("Not a valid PNG file");
    }

    bool first = false;  // Record whether scanned the first chunk -> IHDR

    // For indexed-color images
    uint8_t palette[1024] = {}; // Max 256 colors * 4 bytes (RGBA)
    size_t palette_size = 0;

    // For IDAT chunks
    std::vector<std::vector<uint8_t>> chunk_data;

    struct 
    {
        uint32_t width;
        uint32_t height;
        uint8_t bit_depth;
        uint8_t color_type;
        uint8_t compression_method;
        uint8_t filter_method;
        uint8_t interlace_method;
    } ihdr;

    // Read data chunks
    // Each trunk has the following format:
    // Length (4 bytes) | Chunk Type (4 bytes) | Chunk Data (Length bytes, can be zero) | CRC (4 bytes)
    while (1)
    {
        const auto length = read_u32(ifs, std::endian::big); // length
        const auto type = read_u32(ifs, std::endian::big);   // type

        const auto little = std::byteswap(type);
        std::println("Chunk Type: {}, Length: {}", std::string_view(reinterpret_cast<const char*>(&little), 4), length);

        switch (type)
        {
            // Handle IHDR chunk
            case chunk_type("IHDR"): {

                if (first) 
                {
                    throw std::runtime_error("Multiple IHDR chunks found");
                }

                if (length != 13)
                {
                    throw std::runtime_error("Invalid IHDR chunk length");
                }

                uint8_t ihdr_data[13];
                ifs.read(reinterpret_cast<char*>(ihdr_data), 13);

                const auto width = read_four_bytes(ihdr_data, std::endian::big);
                const auto height = read_four_bytes(ihdr_data + 4, std::endian::big);
                const auto bit_depth = ihdr_data[8];
                const auto color_type = ihdr_data[9];
                const auto compression_method = ihdr_data[10];
                const auto filter_method = ihdr_data[11];
                const auto interlace_method = ihdr_data[12];

                // Check for supported formats
                if (width == 0 || height == 0)
                {
                    throw std::runtime_error("Invalid image dimensions");
                }

                // Gray Scale Image: 1, 2, 4, 8
                // True Color Image: 8, 16
                if (bit_depth != 1 && bit_depth != 2 && bit_depth != 4 && bit_depth != 8 && bit_depth != 16)
                {
                    throw std::runtime_error("Unsupported bit depth, only 1, 2, 4, 8, and 16 are supported");
                }

                // color_type |          mode          | bit_depth
                // ----------------------------------------------------
                //      0     |       Grayscale        | 1, 2, 4, 8, 16
                //      2     |       Truecolor        | 8, 16
                //      3     |      Indexed-color     | 1, 2, 4, 8
                //      4     |  Grayscale with alpha  | 8, 16
                //      6     |  Truecolor with alpha  | 8, 16
                if (color_type > 6 || color_type == 1 || color_type == 5)
                {
                    throw std::runtime_error("Bad color type");
                }

                if (compression_method != 0 || filter_method != 0 || interlace_method > 1)
                {
                    throw std::runtime_error("Unsupported compression method");
                }

                // Note: CRC validation is omitted for brevity
                ifs.seekg(4, std::ios::cur);

                ihdr = { width, height, bit_depth, color_type, compression_method, filter_method, interlace_method };

                std::println("Width: {}, Height: {}, Bit Depth: {}, Color Type: {}, Compression Method: {}, Filter Method: {}, Interlace Method: {}",
                             width, height, bit_depth, color_type, compression_method, filter_method, interlace_method);

            } break;

            // Handle PLTE chunk
            case chunk_type("PLET"): {
                
                if (!first)
                {
                    throw std::runtime_error("PLTE chunk found before IHDR");
                }

                if (ihdr.color_type != 3) // Indexed-color
                {
                    throw std::runtime_error("PLTE chunk found in non-indexed-color image");
                }

                if (palette_size != 0)
                {
                    throw std::runtime_error("Multiple PLTE chunks found");
                }

                // The number of entries is determined from the chunk length. A chunk length not divisible by 3 is an error.
                if (length % 3 != 0 || length > 768 || length == 0) // Max 256 colors * 3 bytes (RGB)
                {
                    throw std::runtime_error("Invalid PLTE chunk length");
                }

                palette_size = length / 3;

                for (size_t i = 0; i < palette_size; ++i)
                {
                    uint8_t rgb[3];
                    ifs.read(reinterpret_cast<char*>(rgb), 3);
                    palette[i * 4 + 0] = rgb[0];
                    palette[i * 4 + 1] = rgb[1];
                    palette[i * 4 + 2] = rgb[2];
                    palette[i * 4 + 3] = 255; // Alpha channel
                }

            } break;

            // Handle IDAT chunk
            case chunk_type("IDAT"): {

                if (!first)
                {
                    throw std::runtime_error("IDAT chunk found before IHDR");
                }

                std::vector<uint8_t> chunk(length);
                ifs.read(reinterpret_cast<char*>(chunk.data()), length);
                chunk_data.push_back(std::move(chunk));
                // Note: CRC validation is omitted for brevity
                ifs.seekg(4, std::ios::cur); // Skip CRC

            } break;

            // Handle IEND chunk
            case chunk_type("IEND"): {

                if (!first)
                {
                    throw std::runtime_error("IEND chunk found before IHDR");
                }

                ifs.seekg(4, std::ios::cur); // Skip chunk data and CRC

                buffer buf;

                // Process IHDR data as needed
                buf.height = ihdr.height;
                buf.width = ihdr.width;

                switch (ihdr.color_type)
                {
                    case 0: buf.channels = 1; break; // Grayscale
                    case 2: buf.channels = 3; break; // Truecolor
                    case 3: buf.channels = 1; break; // Indexed-color
                    case 4: buf.channels = 2; break; // Grayscale with alpha
                    case 6: buf.channels = 4; break; // Truecolor with alpha
                    default: std::unreachable();  // Should never reach here, since we will throw on bad color type above
                }

                // buf.data.resize(width * height * buf.channels * (ihdr.bit_depth / 8));

                return buf;
            };

            default: {
                // Skip unhandled chunk types
                if (!first)
                {
                    throw std::runtime_error("First chunk is not IHDR");
                }

                ifs.seekg(length + 4, std::ios::cur); // Skip chunk data and CRC
            } break;
        }
    
        first = true;
    }
}

} // namespace cpp::images::png

