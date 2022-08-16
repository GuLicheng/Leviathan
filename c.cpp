#include <iostream>
#include <utility>
#include <tuple>
#include <algorithm>

int main()
{
    int values[2];
    std::ranges::sort(values);
    std::tuple<int, double> tp;
    std::tuple_cat(tp, tp);
}








