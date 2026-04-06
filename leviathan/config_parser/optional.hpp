// I try to implement the reference partial version for
// standard std::optional just for reviewing C++
// reference: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2988r12.pdf

#pragma once

#include <optional>
#include <type_traits>

#include <assert.h>

namespace std
{

template <typename T>
class optional<T&>
{

    

};

}