#pragma once

#include <leviathan/collections/core/node_interface.hpp>

namespace cpp::collections
{

struct list_member_hook : list_node_interface
{
    std::array<list_node_interface*, 2> links;

};


}  // namespace cpp::collections
