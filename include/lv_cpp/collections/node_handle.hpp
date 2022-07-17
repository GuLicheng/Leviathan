#pragma once

namespace leviathan::collections
{

    template <typename Iterator, typename NodeType>
    struct node_insert_return
    {
        Iterator position;;
        bool inserted;
        NodeType node;
    };



} // namespace leviathan::collections

