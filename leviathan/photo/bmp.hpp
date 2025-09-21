/*
    BMP file contains four parts:
    1. Bitmap File Header (14 bytes)
    2. Bitmap Information Header (40 bytes)
    3. Color Palette (variable size, optional)
    4. Pixel Data (variable size)
*/
#pragma once

#include "buffer.hpp"
#include "common.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <compare>
#include <algorithm>
#include <optional>
#include <bit>

#include <assert.h>
#include <stdint.h>

#define BI_RGB            0
#define BI_RLE8           1
#define BI_RLE4           2
#define BI_BITFIELDS      3
#define BI_JPEG           4
#define BI_PNG            5
#define BI_ALPHABITFIELDS 6

namespace cpp::image::bmp
{

struct rgbquad
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
};

struct bitmap
{
    static constexpr int FileHeaderSize = 14;
    static constexpr int InfomationHeaderSize = 40;

    // FileHeader
    uint8_t type[2]; // "BM"
    uint32_t size; // size of bmp file
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offbits; // offset (fileheader + info + palette)

    // InfomationHeader
    uint32_t infosize; // 40
    int32_t width;
    int32_t height;
    uint16_t planes; // channel: 1
    uint16_t bitcount;  // 1, 4-16, 8-256, 24-True Color(RGB + α)
    uint32_t compression;
    uint32_t sizeimage;
    int32_t x_pels_per_meter;
    int32_t y_pels_per_meter;
    uint32_t clr_used;
    uint32_t clr_important;

    std::vector<rgbquad> colors;
    std::vector<uint8_t> data;

    buffer to_image_buffer() const
    {
        return {
            .height = height,
            .width = width,
            .channels = bitcount / 8,
            .data = data
        };
    }

};

inline int padding(const bitmap& info)
{
    int C = info.bitcount >> 3;
    return 4 - (C * info.width) & 3;
}

inline constexpr bool check_support(const bitmap& info)
{
    return info.compression == BI_RGB
        && (info.bitcount == 8 || info.bitcount == 24);
}

inline constexpr check_header(const bitmap& info)
{
    return info.type[0] == 'B' and info.type[1] == 'M'
        && info.infosize == 40
        && info.planes == 1
        && info.offbits == info.InfomationHeaderSize + info.FileHeaderSize + info.clr_used * 4
        && info.size == info.offbits + info.sizeimage
        && info.sizeimage == (info.width * 3 + padding(info)) * info.height;
}

class reader
{

public:

    static buffer read(const char* filename)
    {
        std::ifstream fs;

        fs.open(filename, std::ios::in | std::ios::binary);

        if (!fs.is_open())
        {
            throw std::runtime_error("File Error");
        }

        bitmap info;

        // Read FileHeader
        uint8_t header_buffer[bitmap::FileHeaderSize];
        fs.read(reinterpret_cast<char*>(header_buffer), bitmap::FileHeaderSize);

        info.type[0] = header_buffer[0];
        info.type[1] = header_buffer[1];
        info.size = read_four_bytes(header_buffer + 2);
        info.reserved1 = read_two_bytes(header_buffer + 6);
        info.reserved2 = read_two_bytes(header_buffer + 8);
        info.offbits = read_four_bytes(header_buffer + 10);

        // Read InfomationHeader
        uint8_t infomation_buffer[bitmap::InfomationHeaderSize];
        fs.read(reinterpret_cast<char*>(infomation_buffer), bitmap::InfomationHeaderSize);
        info.infosize = read_four_bytes(infomation_buffer);
        info.width = (int32_t)read_four_bytes(infomation_buffer + 4);
        info.height = (int32_t)read_four_bytes(infomation_buffer + 8);
        info.planes = read_two_bytes(infomation_buffer + 12);
        info.bitcount = read_two_bytes(infomation_buffer + 14);
        info.compression = read_four_bytes(infomation_buffer + 16);
        info.sizeimage = read_four_bytes(infomation_buffer + 20);
        info.x_pels_per_meter = (int32_t)read_four_bytes(infomation_buffer + 24);
        info.y_pels_per_meter = (int32_t)read_four_bytes(infomation_buffer + 28);
        info.clr_used = read_four_bytes(infomation_buffer + 32);
        info.clr_important = read_four_bytes(infomation_buffer + 36);

        if (!check_header(info) || !check_support(info))
        {
            throw std::runtime_error("Not a valid BMP file");
        }

        // Read Palette
        if (info.clr_used)
        {
            info.colors.resize(info.clr_used);
            fs.read(reinterpret_cast<char*>(info.colors.data()), info.clr_used * 4);
        }

        // Read Pixel Data
        int W = info.width, H = info.height;
        const int pad = padding(info);
        const int data_line_byte = info.bitcount / 8 * W;
        info.data.resize(data_line_byte * H);
        auto dest = info.data.data();

        for (int y = 0; y < H; ++y, dest += data_line_byte)
        {
            fs.read(reinterpret_cast<char*>(dest), data_line_byte);
            fs.ignore(pad);
        }

        return info.to_image_buffer();
    }
};

class writer
{
public:

    static void write(const char* filename, const buffer& buf)
    {
        std::ofstream fs;
        fs.open(filename, std::ios::out | std::ios::binary);

        if (!fs.is_open())
        {
            return;
        }

        bitmap info;

        info.type[0] = 'B';
        info.type[1] = 'M';
        info.infosize = bitmap::InfomationHeaderSize;
        info.width = buf.width;
        info.height = buf.height;
        info.planes = 1;
        info.bitcount = buf.channels * 8;
        info.compression = BI_RGB;
        info.x_pels_per_meter = 0;
        info.y_pels_per_meter = 0;
        info.clr_used = 0;
        info.clr_important = 0;

        const int pad = padding(info);
        const int data_line_byte = info.bitcount / 8 * info.width;
        info.sizeimage = (data_line_byte + pad) * info.height;
        info.clr_used = 0;
        info.offbits = bitmap::FileHeaderSize + bitmap::InfomationHeaderSize + info.clr_used * 4;
        info.size = info.offbits + info.sizeimage;

        // Pixel Data
        const auto* src = buf.data.data();
        info.data.resize(info.sizeimage);
        auto* dest = info.data.data();

        for (int y = 0; y < buf.height; ++y, src += data_line_byte)
        {
            std::copy(src, src + data_line_byte, dest);
            dest += data_line_byte;
            std::fill(dest, dest + pad, 0);
            dest += pad;
        }

        // Write FileHeader
        fs.write(reinterpret_cast<const char*>(info.type), 2);
        fs.write(reinterpret_cast<const char*>(&info.size), 4);
        fs.write(reinterpret_cast<const char*>(&info.reserved1), 2);
        fs.write(reinterpret_cast<const char*>(&info.reserved2), 2);
        fs.write(reinterpret_cast<const char*>(&info.offbits), 4);

        // Write InfomationHeader
        fs.write(reinterpret_cast<const char*>(&info.infosize), 4);
        fs.write(reinterpret_cast<const char*>(&info.width), 4);
        fs.write(reinterpret_cast<const char*>(&info.height), 4);
        fs.write(reinterpret_cast<const char*>(&info.planes), 2);
        fs.write(reinterpret_cast<const char*>(&info.bitcount), 2);
        fs.write(reinterpret_cast<const char*>(&info.compression), 4);
        fs.write(reinterpret_cast<const char*>(&info.sizeimage), 4);
        fs.write(reinterpret_cast<const char*>(&info.x_pels_per_meter), 4);
        fs.write(reinterpret_cast<const char*>(&info.y_pels_per_meter), 4);
        fs.write(reinterpret_cast<const char*>(&info.clr_used), 4);
        fs.write(reinterpret_cast<const char*>(&info.clr_important), 4);

        // Write Palette
        for (const auto& color : info.colors)
        {
            fs.write(reinterpret_cast<const char*>(&color), 4);
        }

        // Write Pixel Data
        int W = info.width, H = info.height;
        const uint8_t padding_bytes[3] = {0, 0, 0};
        src = info.data.data();

        for (int y = 0; y < H; ++y, src += data_line_byte)
        {
            fs.write(reinterpret_cast<const char*>(src), data_line_byte);
            fs.write(reinterpret_cast<const char*>(padding_bytes), pad);
        }
    }
};

}