#include <string>
#include <iostream>
#include <tuple>

template <typename Target, typename Fields>
class build_impl;

template <typename Target, typename... Fields>
class build_impl<Target, std::tuple<Fields...>>
{

};


int main()
{
    std::tuple<std::string> t("This sentence must be longer enough.");
    auto [s] = std::move(t);
    std::cout << s << '\n';
    std::cout << std::get<0>(t) << '\n';
}

