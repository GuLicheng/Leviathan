#include <set>
#include <iostream>

template <typename T>
struct Base
{
    template <bool B> 
    using key_arg_t = std::conditional_t<B, std::true_type, std::false_type>;
};

template <typename T>
struct Derived : Base<T>
{
    // using type = Base<T>::template key_arg_t<true>;

    template <bool B>
    using key_arg_t = Base<T>::template key_arg_t<B>;

    using type = key_arg_t<true>;
};

int main()
{



    std::cout << "Ok\n";

}
