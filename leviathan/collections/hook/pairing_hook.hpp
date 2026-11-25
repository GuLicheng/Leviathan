#pragma once

#include <leviathan/collections/core/node_interface.hpp>

namespace cpp::collections::intrusive
{

struct pairing_hook : node_interface
{
    pairing_hook* links[3];

    template <typename Compare>
    constexpr pairing_hook* link(pairing_hook* other, Compare comp)
    {
        if (!other) return this;

        auto x = this;
        auto y = other;

        if (comp(x, y))
        {
            std::swap(x, y);
        }

        y->sibling(x->child());
        if (x->child()) x->child()->sibling(y);
        x->child(y);
        y->parent(x);

        return x;
    }

};

}  // namespace cpp::collections::intrusive
