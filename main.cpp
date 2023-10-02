#include <iostream>
#include <any>
#include <cstdint>
#include <memory>
#include <optional>

using T = std::optional<int>;

int main(int argc, char const *argv[])
{

    using U1 = std::pointer_traits<T>::element_type;
    using U2 = std::pointer_traits<T>::pointer;
    return 0;
}

