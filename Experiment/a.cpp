#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <bit>

enum Endian {
  Big, Little
};

template <Endian E> struct png;

template <>
struct png<Endian::Little>
{
  constexpr static uint64_t prefix_identifier = 0x89504e460d0a1a0a; // 8bytes
};


void read_png_image(const char *path)
{
  std::ifstream fs{ path, std::ios::in | std::ios::binary };
  std::string buffer = {std::istreambuf_iterator<char>(fs), 
                        std::istreambuf_iterator<char>()};
  std::cout << "buffer.len = " << buffer.size() << '\n';
  std::byte bytes[8];
  for (int i = 0; i < 8; ++i) 
  {
    bytes[i] = (std::byte)buffer[i];
    std::cout << std::hex << (int)bytes[i] << ' ';
  }
  return;
}



int main(int argc, char const *argv[])
{
  const char *path = "D:\\Library\\Leviathan\\Experiment\\demo.png";
  read_png_image(path);
  return 0;
}
