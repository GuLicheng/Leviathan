#pragma once

#include <leviathan/collections/core/node_interface.hpp>

namespace cpp::collections::intrusive
{

struct bs_hook : tree_node_interface
{
    bs_hook* links[3];

    constexpr bool is_sentinel() const
    {
        return false;
    } 

    constexpr void as_sentinel()
    { }

    constexpr void link(bool insert_left, bs_hook* p, bs_hook& header)
    {
        this->insert_node_and_update_header(insert_left, p, header);
    }

    constexpr void unlink(bs_hook& header)
    {
        this->replace_node_with_successor(header);
    }
};

}  // namespace cpp::collections::intrusive

