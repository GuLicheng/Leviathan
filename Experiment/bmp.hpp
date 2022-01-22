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
#include <lv_cpp/algorithm/sort.hpp>

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

namespace tag
{
    struct BMP { };
};

struct little_endian
{
    static uint32_t read_four_bytes(const uint8_t* p)  
    {
       return p[0]     
           | (p[1] << 8)
           | (p[2] << 16)
           | (p[3] << 24);
    }

    static uint16_t read_two_bytes(const uint8_t* p)
    {
        return p[0] | (p[1] << 8);
    }
};


struct bmp_info
{
    constexpr static int FileHeaderSize = 14;
    constexpr static int InfomationHeaderSize = 40;
    
    // FileHeader
    uint8_t bfType[2]; // "BM"
    uint32_t bfSize; // size of bmp file
    uint8_t bfReserved[4]; // useless
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
        uint8_t blue;
        uint8_t red;
        uint8_t green;
        uint8_t reserved = 0;
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


};

template <typename Endian> 
struct bmp
{

    using image_type = tag::BMP;

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

    bool read(const char* filename) 
    {
        std::ifstream fs;
        fs.open(filename, std::ios::in | std::ios::binary);
        if (!fs.is_open())  
            return false;
    
        // bmp_info info;

        uint8_t headerbuffer[bmp_info::FileHeaderSize];
        fs.read(reinterpret_cast<char*>(headerbuffer), bmp_info::FileHeaderSize);

        // for (auto byte : headerbuffer)
        //     std::cout <<  std::hex << (int)byte << ' ';
        // std::endl(std::cout);

        info.bfType[0] = headerbuffer[0];
        info.bfType[1] = headerbuffer[1];
        info.bfSize = Endian::read_four_bytes(headerbuffer + 2);
        info.bfOffbits = Endian::read_four_bytes(headerbuffer + 10);

        uint8_t infomation_buffer[bmp_info::InfomationHeaderSize];
        fs.read(reinterpret_cast<char*>(infomation_buffer), bmp_info::InfomationHeaderSize);

        // for (auto byte : infomation_buffer)
        //     std::cout <<  std::hex << (int)byte << ' ';
        // std::endl(std::cout);

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
        const int padding = ((W * info.biBitCount + 31) >> 5) << 2;  
        info.biData.resize(info.biSizeImage);
        for (int y = 0; y < H; ++y)
        {
            for (int x = 0; x < W; ++x)
            {
                uint8_t color[3];
                fs.read(reinterpret_cast<char*>(color), 3);
                info.biData[y * W + x + 0] = color[0];
                info.biData[y * W + x + 1] = color[1];
                info.biData[y * W + x + 2] = color[2];
            }
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

        std::cout << "Simple Write FileHeader\n";

        // Simple Write FileHeader
        fs.write(reinterpret_cast<char*>(&info.bfType), 2);
        fs.write(reinterpret_cast<char*>(&info.bfSize), 4);
        fs.write(reinterpret_cast<char*>(&info.bfReserved), 4);
        fs.write(reinterpret_cast<char*>(&info.bfOffbits), 4);

        std::cout << "Simple Write InformationHeader\n";
        // Simple Write InformationHeader
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

        std::cout << "Here\n";
        // Simple Write Palette
        for (auto byte : info.bmiColors)
            fs.write(reinterpret_cast<char*>(&byte), 4);
        
        // Simple Write Data
        int W = info.biWidth, H = info.biHeight;
        const int padding = ((W * info.biBitCount + 31) >> 5) << 2;  
        int padding_val[padding] = { };
        for (int y = 0; y < H; ++y)
        {
            for (int x = 0; x < W; ++x)
            {
                uint8_t color[3];
                color[0] = info.biData[y * W + x + 0];
                color[1] = info.biData[y * W + x + 1];
                color[2] = info.biData[y * W + x + 2];
                fs.write(reinterpret_cast<char*>(color), 3);
            }
            fs.write(reinterpret_cast<char*>(padding_val), padding);
        }
        fs.close();
    }

};
