#include <iostream>
#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <deque>

using T = std::optional<int>;

int main(int argc, char const *argv[])
{
    std::deque<int> de;
    using U1 = std::pointer_traits<T>::element_type;
    using U2 = std::pointer_traits<T>::pointer;
    return 0;
}

