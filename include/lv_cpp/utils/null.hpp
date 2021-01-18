#ifndef __NULL_HPP__
#define __NULL_HPP__

#include <iostream> // for debug

namespace leviathan
{
// helper class, used as instance of void
struct null 
{
    null(...) { }
};

std::ostream& operator<<(std::ostream& os, [[maybe_unused]]const null& n)
{
    return os << "null";
}

}

#endif