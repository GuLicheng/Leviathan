#include <leviathan/collections/list/skip_list.hpp>
#include <leviathan/print.hpp>

using namespace leviathan::collections;

struct F
{
    int* X = new int(1);

    template <typename Self>
    copy_const_t<Self, int*> GetX(this Self& self) 
    {
        return self.X;
    }
};

int main(int argc, char const *argv[])
{
    const F f;
    auto c = f.GetX();
    auto d = f.X;
    *c = 2;
    Console::WriteLine(*f.X);

    return 0;
}
//     