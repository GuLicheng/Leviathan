#include "bmp.hpp"


int main()
{
    const char* path1 = R"(D:\Library\Leviathan\Experiment\20140514114029140.bmp)";
    const char* path2 = R"(D:\Library\Leviathan\Experiment\lena1.bmp)";
    bmp<little_endian> b1, b2;
    std::cout << "==============================================\n";
    b2.read("./PC.bmp");
    b2.info.display();
    b2.save("./PC1.bmp");
    b1.read("./PC1.bmp");
    std::cout << (b1.info.biData == b2.info.biData) << '\n';
    std::cout << (b1.info.bmiColors == b2.info.bmiColors) << '\n';
    std::cout << (b1 == b2) << '\n';
    std::cout << "==============================================\n";
}
