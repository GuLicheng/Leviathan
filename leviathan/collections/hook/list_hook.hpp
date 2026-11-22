#pragma once

#include <leviathan/collections/core/node_interface.hpp>
#include <leviathan/collections/core/container_interface.hpp>
#include <leviathan/collections/common.hpp>

namespace cpp::collections::intrusive
{

struct list_hook : list_node_interface
{
    list_hook* links[2];
};

}  // namespace cpp::collections::intrusive
