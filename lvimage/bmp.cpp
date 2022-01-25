#include "bmp.hpp"


#include <filesystem>

namespace fs = std::filesystem;


int main(int argc, char const *argv[])
{
    using namespace leviathan::image;
    assert(argc == 3);
    const char* src = argv[1];
    const char* dest = argv[2];
    bmp b;
    std::cout << "==============================================\n";
    b.read(src);
    b.info.display();
    b.write(dest);
    std::cout << "==============================================\n";
}
