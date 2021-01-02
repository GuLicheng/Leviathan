#ifndef _IMG_READER_HPP_
#define _IMG_READER_HPP_

#include <fstream>
#include <string>
#include <string_view>
#include <ranges>


template <typename T = char>
class img_reader
{
    public:
        img_reader(const T* path) 
            : in{path, std::basic_ifstream<T>::in | std::ios::binary } { }
        
        img_reader(const std::basic_string<T>& path) 
            : in{path.data(), std::basic_ifstream<T>::in | std::ios::binary } { }
    
        img_reader(const std::basic_string_view<T>& path)
            : in{path.data(), std::basic_ifstream<T>::in | std::ios::binary } { }

        /*
            for std::array, std::vector and inner array
        */
        template <std::ranges::contiguous_range Rng>
        void read(Rng&& rng)
        {
            in.seekg(0, in.end);
            const int length = in.tellg();
            in.seekg(0, in.beg);
            if constexpr (requires (Rng r) { r.resize(); }) rng.resize(length);
            in.read((T*)rng.data(), length);
        }
    
    private:
        std::basic_ifstream<T> in;
};




#endif



/*

void test()
{
    std::string str = R"(D:\Pictures\small.jpg)";
    img_reader reader(str);
    std::vector<uint8_t> vec;
    // std::array<uint8_t, 1587> vec;
    reader.read(vec);
    std::copy(vec.cbegin(), vec.cend(), std::ostream_iterator<int>{std::cout, " "});
}

*/