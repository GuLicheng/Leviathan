#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <print>
#include <meta>


int main()
{

    auto fn = cpp::select_tuple_element<>;

    auto t1 = std::make_tuple(1, 2, 3);
    auto t2 = fn(t1);
    std::println("t2 = {}", t2);
}
