#pragma once

#include <leviathan/collections/core/node_interface.hpp>

namespace cpp::collections::intrusive
{

struct pairing_hook : node_interface
{
    pairing_hook* links[3];

    

};

}  // namespace cpp::collections::intrusive
