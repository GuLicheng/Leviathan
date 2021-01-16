#include <lv_cpp/math/matrix.hpp>

using namespace leviathan;

void test3()
{
     leviathan::numeric::matrix<double> m = 
    {
        {1, 2, 3},
        {4, 5, 6}
    };

     leviathan::numeric::matrix<double> m0 = 
    {
        {1, 2},
        {3, 4}
    };
    std::cout << (m0 * m0) << std::endl;
    auto m1 = 1 + (m0 * m0) + 2;
    std::cout << m1 << std::endl;
    std::cout << (m1 - 1) << std::endl;
    std::cout << (1 - m1) << std::endl;
    std::cout << m1 * 2 << std::endl;
    std::cout << 2 * m1 << std::endl;
    std::cout << 2 / m1 << std::endl;
    std::cout << m1 / 2 << std::endl;
}



void test2()
{
    try
    {
        std::cout << "===========================\n";
        // constexpr auto a = std::numeric_limits<double>::max_digits10;
        std::vector arr{1, 2, 3}, buf{4, 5, 6};
         leviathan::numeric::matrix<double> m1{arr, buf};
         leviathan::numeric::matrix<double> m2
        {
            {1, 2, 3},
            {3, 4, 5}
        };
        std::cout << m1 << std::endl;
        std::cout << m2 << std::endl;
        auto m3 = m1;
        std::cout << m3 << std::endl;
        m3 = buf;
        std::cout << m3 << std::endl;
        std::cout << m3.transpose() << std::endl;
        m2 -= m2;
        std::cout << m2 << std::endl;
        m2 += 1;
        m2 *= 2;
        std::cout << m2 << std::endl;


         leviathan::numeric::matrix<double> m0 =
        {
            {1, 2},
            {3, 4}
        };

        // m0 *= m0;
        // std::cout << m0 << std::endl;

        // std::cout << m0 + m0 << std::endl;

    } 
    catch (std::string x)
    {
        std::cout << x;
    }
}

int main()
{

    test2();
    test3();
}
