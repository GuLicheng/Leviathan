#include <iostream>
#include <utility>
#include <tuple>


int main()
{
    std::tuple<int, double> tp;
    std::tuple_cat(tp, tp);
}








