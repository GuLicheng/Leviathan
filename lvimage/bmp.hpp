#pragma once


/*
    Reference:
            https://blog.csdn.net/qingchuwudi/article/details/25785307
            https://zh.wikipedia.org/wiki/BMP
            https://www.cnblogs.com/pang1567/p/3671474.html
*/

#include "base.hpp"

namespace leviathan::image 
{

#define BI_RGB            0
#define BI_RLE8           1
#define BI_RLE4           2
#define BI_BITFIELDS      3
#define BI_JPEG           4
#define BI_PNG            5
#define BI_ALPHABITFIELDS 6

struct bmp_info
{
    static constexpr int FileHeaderSize = 14;
    static constexpr int InfomationHeaderSize = 40;
    
    // FileHeader
    uint8_t bfType[2]; // "BM"
    uint32_t bfSize; // size of bmp file
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffbits; // offset (fileheader + info + palette)
    
    // InfomationHeader
    uint32_t biSize; // 40
    int32_t biWidth;  
    int32_t biHeight;
    uint16_t biPlanes; // channel: 1
    uint16_t biBitCount;  // 1, 4-16, 8-256, 24-True Color(RGB + α)
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;


    struct rgbquad
    {
        uint8_t rgbBlue;
        uint8_t rgbGreen;
        uint8_t rgbRed;
        uint8_t rgbReserved;
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

struct bmp
{

    using image_type = tag::BMP;
    static constexpr auto endian = std::endian::native;

private:

    bool check_support() const 
    {
        assert(info.biCompression == BI_RGB);
        assert(info.biBitCount == 8 || info.biBitCount == 24);
        return true;
    }

    bool check_header() const
    {
        assert(info.bfType[0] == 'B' and info.bfType[1] == 'M');
        assert(info.biSize == 40);
        assert(info.biPlanes == 1);
        assert(info.bfOffbits == bmp_info::InfomationHeaderSize + bmp_info::FileHeaderSize + info.biClrUsed * 4);
        assert(info.bfSize == info.bfOffbits + info.biSizeImage);
        assert(info.biSizeImage == (info.biWidth * 3 + padding()) * info.biHeight);
        // println(std::cout, padding());
        return true;
    }

    int padding() const 
    {
        int C = info.biBitCount >> 3;
        return 4 - (C * info.biWidth) & 3;
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
        info.bfSize = read_four_bytes(headerbuffer + 2);
        info.bfOffbits = read_four_bytes(headerbuffer + 10);

        // Read infomation
        uint8_t infomation_buffer[bmp_info::InfomationHeaderSize];
        fs.read(reinterpret_cast<char*>(infomation_buffer), bmp_info::InfomationHeaderSize);

        info.biSize = read_four_bytes(infomation_buffer);

        auto W = (int32_t)read_four_bytes(infomation_buffer + 4);
        auto H = (int32_t)read_four_bytes(infomation_buffer + 8);
        info.biWidth = W, info.biHeight = H;

        info.biPlanes = read_two_bytes(infomation_buffer + 12);
        info.biBitCount = read_two_bytes(infomation_buffer + 14);
        info.biCompression = read_four_bytes(infomation_buffer + 16);
        info.biSizeImage = read_four_bytes(infomation_buffer + 20);
        info.biXPelsPerMeter = (int32_t)read_four_bytes(infomation_buffer + 24);
        info.biYPelsPerMeter = (int32_t)read_four_bytes(infomation_buffer + 28);
        info.biClrUsed = read_four_bytes(infomation_buffer + 32);
        info.biClrImportant = read_four_bytes(infomation_buffer + 36);

        // info.display();
        check_header();

        // Read palette
        if (info.biClrUsed)
        {
            info.bmiColors.resize(info.biClrUsed);
            fs.read(reinterpret_cast<char*>(info.bmiColors.data()), info.biClrUsed * 4);
        }
        // Read pixel
        const int pad = padding();
        const int data_line_byte = info.biBitCount / 8 * W;
        info.biData.resize(data_line_byte * H);
        auto dest = info.biData.data();
        for (int y = 0; y < H; ++y, dest += data_line_byte)
        {
            fs.read(reinterpret_cast<char*>(dest), data_line_byte);
            fs.ignore(pad);
        }

        fs.close();
        return check_support();
    }

    void write(const char* filename)
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
        const int pad = padding();
        const int data_line_byte = info.biBitCount / 8 * W;
        uint8_t padding_val[] = { 0, 0, 0 };
        const auto* src = info.biData.data();
        for (int y = 0; y < H; ++y, src += data_line_byte)
        {
            fs.write(reinterpret_cast<const char*>(src), data_line_byte);
            fs.write(reinterpret_cast<char*>(padding_val), pad);
        }

        fs.close();
    }

    image_buffer buffer()
    {
        return {
            .m_height = info.biHeight,
            .m_width = info.biWidth,
            .m_channels = info.biBitCount / 8,
            .m_data = info.biData
        };
    }

};

} // namespace 