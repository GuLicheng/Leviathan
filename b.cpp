#include <lv_cpp/collections/skip_list.hpp>

leviathan::skip_list<int> s1;

using T = const leviathan::skip_list<int>;

#define Line() (::putchar('\n'))

void copy_test()
{
    leviathan::skip_list<int> s2(s1);
    s2 = s1;
    s2.show();
    Line();
}

void move_test()
{
    leviathan::skip_list<int> s3(std::move(s1));
    s3.show();
    Line();
}

int main()
{
    for (int i = 0; i < 10; ++i) s1.insert(i);
    copy_test();
    // move_test();
    std::cout << "OK\n";
}