#ifndef __NULL_HPP__
#define __NULL_HPP__

#include <iostream> // for debug

namespace leviathan
{
    // helper class, used as instance of void
    struct null
    {
        null(...) {}
    };

    std::ostream &operator<<(std::ostream &os, const null &)
    {
        return os << "null";
    }

    // Some empty class
    struct monostate { }; // header<variant>

    struct none_t { };
    struct sentinel_t { };
    struct no_init_t { };
    struct in_place_t { };

    // Some instance
    inline constexpr none_t none { };
    inline constexpr sentinel_t sentinel { };
    inline constexpr no_init_t no_init { };
    inline constexpr in_place_t in_place { };

}

#endif