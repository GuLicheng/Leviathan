#include <lv_cpp/collections/bitmap.hpp>

using namespace leviathan::collections;

void test_one_bit()
{
    bitmap<1> map1;
    map1[0] = 0;
    map1[1] = 1;
    map1[2] = 1;

    map1.display();
    for (int i = 0; i < 10; ++i) 
    {
        std::cout << map1[i] << ' ';
    }
    std::cout << '\n';
}

void test_two_bit()
{
    bitmap<2> map2;

    map2[0] = 0;
    map2[1] = 1;
    map2[2] = 2;
    map2[3] = 3;

    map2.display();
    for (int i = 0; i < 5; ++i) 
    {
        std::cout << map2[i] << ' ';
    }
    std::cout <<  '\n';
}

int main()
{
    test_one_bit();
    test_two_bit();

}
