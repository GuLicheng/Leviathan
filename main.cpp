#include <iostream>

struct B
{
    template <typename Self>
    void do_(this Self&& self, int i) { std::cout << "Base\n"; }
};

struct D : B
{
    using B::do_;
    void do_(double i) { std::cout << "Base\n"; }
};

int main(int argc, char const *argv[])
{

    D d;

    d.do_(1);

}
