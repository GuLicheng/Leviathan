/*
    Reference:
            https://blog.csdn.net/qingchuwudi/article/details/25785307
            https://zh.wikipedia.org/wiki/BMP
            https://www.cnblogs.com/pang1567/p/3671474.html
*/
#pragma once

#include <assert.h>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <compare>
#include <ranges>

template <typename OStream, typename... Ts>
void println(OStream& os, const Ts&... ts)
{
    (os << ... << ts) << std::endl;
}

#define BI_RGB            0
#define BI_RLE8           1
#define BI_RLE4           2
#define BI_BITFIELDS      3
#define BI_JPEG           4
#define BI_PNG            5
#define BI_ALPHABITFIELDS 6



struct bmp_info
{
    constexpr static int FileHeaderSize = 14;
    constexpr static int InfomationHeaderSize = 40;
    
    // FileHeader
    uint8_t bfType[2]; // "BM"
    uint32_t bfSize; // size of bmp file
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffbits; // offset (fileheader + info + palette)
    
    // InfomationHeader
    uint32_t biSize; // 40
    uint32_t biWidth;  
    uint32_t biHeight;
    uint16_t biPlanes; // channel: 1
    uint16_t biBitCount;  // 1, 4-16, 8-256, 24-True Color(RGB + α)
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;


    struct rgbquad
    {
        uint8_t rgbBlue;
        uint8_t rgbGreen;
        uint8_t rgbRed;
        uint8_t rgbReserved;
        auto operator<=>(const rgbquad&) const = default;
    };

    std::vector<rgbquad> bmiColors;

    std::vector<uint8_t> biData;


    void display() const
    {
        std::cout << std::dec;
        println(std::cout, "bfType: ", bfType[0], ' ', bfType[1]);

#define PrintFiled(name) println(std::cout, #name, " :", name)

        PrintFiled(bfSize);
        PrintFiled(bfOffbits);
        PrintFiled(biSize);
        PrintFiled(biWidth);
        PrintFiled(biHeight);
        PrintFiled(biPlanes);
        PrintFiled(biBitCount);
        PrintFiled(biCompression);
        PrintFiled(biSizeImage);
        PrintFiled(biXPelsPerMeter);
        PrintFiled(biYPelsPerMeter);
        PrintFiled(biClrUsed);
        PrintFiled(biClrImportant);

#undef PrintFiled
        std::cout << "palette.size = " << bmiColors.size() << '\n';
        std::cout << "data.size = " << biData.size() << '\n';

    }

    auto operator<=>(const bmp_info&) const = default;
};

template <typename Endian> 
struct bmp
{

    using image_type = tag::BMP;
    using endian_type = Endian;

private:
    bool check() const 
    {
        if (info.bfType[0] != 'B' || info.bfType[1] != 'M')
            return false;
        
        assert(info.biSize == 40);
        assert(info.biPlanes == 1);
        assert(info.bfOffbits == bmp_info::InfomationHeaderSize + bmp_info::FileHeaderSize + info.biClrUsed * sizeof(bmp_info::rgbquad));
        assert(info.bfSize == info.bfOffbits + info.biSizeImage);

        assert(info.biCompression == BI_RGB);
        assert(info.biBitCount == 8 || info.biBitCount == 24);
        return true;
    }
public:

    bmp_info info;

    auto operator<=>(const bmp&) const = default;

    bool read(const char* filename) 
    {
        std::ifstream fs;
        fs.open(filename, std::ios::in | std::ios::binary);
        if (!fs.is_open())  
            return false;
    
        // Read header
        uint8_t headerbuffer[bmp_info::FileHeaderSize];
        fs.read(reinterpret_cast<char*>(headerbuffer), bmp_info::FileHeaderSize);

        info.bfType[0] = headerbuffer[0];
        info.bfType[1] = headerbuffer[1];
        info.bfSize = Endian::read_four_bytes(headerbuffer + 2);
        info.bfOffbits = Endian::read_four_bytes(headerbuffer + 10);

        // Read infomation
        uint8_t infomation_buffer[bmp_info::InfomationHeaderSize];
        fs.read(reinterpret_cast<char*>(infomation_buffer), bmp_info::InfomationHeaderSize);

        info.biSize = Endian::read_four_bytes(infomation_buffer);

        uint32_t W = Endian::read_four_bytes(infomation_buffer + 4);
        uint32_t H = Endian::read_four_bytes(infomation_buffer + 8);
        info.biWidth = W, info.biHeight = H;

        info.biPlanes = Endian::read_two_bytes(infomation_buffer + 12);
        info.biBitCount = Endian::read_two_bytes(infomation_buffer + 14);
        info.biCompression = Endian::read_four_bytes(infomation_buffer + 16);
        info.biSizeImage = Endian::read_four_bytes(infomation_buffer + 20);
        info.biXPelsPerMeter = Endian::read_four_bytes(infomation_buffer + 24);
        info.biYPelsPerMeter = Endian::read_four_bytes(infomation_buffer + 28);
        info.biClrUsed = Endian::read_four_bytes(infomation_buffer + 32);
        info.biClrImportant = Endian::read_four_bytes(infomation_buffer + 36);

        // Read palette
        if (info.biClrUsed)
        {
            info.bmiColors.reserve(info.biClrUsed);
            for (int i = 0; i < info.biClrUsed; ++i)
            {
                uint8_t color[4];
                fs.read(reinterpret_cast<char*>(color), 4);
                info.bmiColors.emplace_back(color[0], color[1], color[2], color[3]);
            }
        }

        // Read pixel
        const int padding = 4 - ((W * info.biBitCount)>>3) & 3;
        const int data_line_byte = info.biBitCount * W;
        info.biData.resize(info.biSizeImage);
        auto dest = info.biData.data();
        for (int y = 0; y < H; ++y, dest += data_line_byte)
        {
            // for (int x = 0; x < W; ++x)
            // {
            //     uint8_t color[3];
            //     fs.read(reinterpret_cast<char*>(color), 3);
            //     *dest ++ = color[0];
            //     *dest ++ = color[1];
            //     *dest ++ = color[2];
            // }
            fs.read(reinterpret_cast<char*>(dest), data_line_byte);
            fs.ignore(padding);
        }

        fs.close();
        return check();
    }


    void save(const char* filename)
    {
        std::ofstream fs;
        fs.open(filename, std::ios::out | std::ios::binary);
        if (!fs.is_open())
            std::cerr << "File Error\n";


        // Simplely Write FileHeader
        fs.write(reinterpret_cast<char*>(&info.bfType), 2);
        fs.write(reinterpret_cast<char*>(&info.bfSize), 4);
        fs.write(reinterpret_cast<char*>(&info.bfReserved1), 2);
        fs.write(reinterpret_cast<char*>(&info.bfReserved2), 2);
        fs.write(reinterpret_cast<char*>(&info.bfOffbits), 4);

        // Simplely Write InformationHeader
        fs.write(reinterpret_cast<char*>(&info.biSize), 4);
        fs.write(reinterpret_cast<char*>(&info.biWidth), 4);
        fs.write(reinterpret_cast<char*>(&info.biHeight), 4);
        fs.write(reinterpret_cast<char*>(&info.biPlanes), 2);
        fs.write(reinterpret_cast<char*>(&info.biBitCount), 2);
        fs.write(reinterpret_cast<char*>(&info.biCompression), 4);
        fs.write(reinterpret_cast<char*>(&info.biSizeImage), 4);
        fs.write(reinterpret_cast<char*>(&info.biXPelsPerMeter), 4);
        fs.write(reinterpret_cast<char*>(&info.biYPelsPerMeter), 4);
        fs.write(reinterpret_cast<char*>(&info.biClrUsed), 4);
        fs.write(reinterpret_cast<char*>(&info.biClrImportant), 4);

        // Simplely Write Palette
        for (auto byte : info.bmiColors)
            fs.write(reinterpret_cast<char*>(&byte), 4);
        
        // Simple Write Data
        int W = info.biWidth, H = info.biHeight;
        const int padding =  4 - ((W * info.biBitCount)>>3) & 3; 
        const int data_line_byte = info.biBitCount * W;
        uint8_t padding_val[] = { 0, 0, 0 };
        const auto* src = info.biData.data();
        for (int y = 0; y < H; ++y, src += data_line_byte)
        {
            // for (int x = 0; x < W; ++x)
            // {
            //     uint8_t color[3];
            //     color[0] = *src++;
            //     color[1] = *src++;
            //     color[2] = *src++;
            //     fs.write(reinterpret_cast<char*>(color), 3);
            // }
            fs.write(reinterpret_cast<const char*>(src), data_line_byte);
            fs.write(reinterpret_cast<char*>(padding_val), padding);
        }

        fs.close();
    }


    bmp_info default_header()
    {
        bmp_info i;

    }

};
