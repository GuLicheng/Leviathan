#include "bmp.hpp"


int main()
{
    const char* path1 = R"(D:\Library\Leviathan\Experiment\20140514114029140.bmp)";
    const char* path2 = R"(D:\Library\Leviathan\Experiment\PC.bmp)";
    bmp<little_endian> b1, b2;
    b1.read(path1);
    b1.info.display();
    std::cout << "==============================================\n";
    b2.read(path2);
    b2.info.display();
    b2.save("./lena.bmp");
    std::cout << "==============================================\n";
}
