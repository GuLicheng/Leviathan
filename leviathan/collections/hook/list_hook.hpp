#pragma once

#include <leviathan/collections/core/node_interface.hpp>

namespace cpp::collections::intrusive
{

struct list_hook : list_node_interface
{
    list_hook* links[2];
};

template <> 
struct mermaid_render<list_hook>
{
    template <typename ToString>
    static constexpr std::string operator()(const list_hook* node, ToString to_string)
    {
        throw std::runtime_error("Mermaid rendering not implemented for list_hook");
    }
};


}  // namespace cpp::collections::intrusive
