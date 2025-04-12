#pragma once

#include "tree_node_operation.hpp"

namespace cpp::collections
{

struct binary_node : binary_node_operation
{
    // Nodes
    binary_node* m_link[3];
    
    // True for header and false for others
    bool m_sentinel;

    // Initialize node without value field after calling allocate
    void init()
    {
        m_sentinel = false;
    }

    // Reset header for an empty tree 
    void as_empty_tree_header()
    {
        m_sentinel = true;
    }

    bool is_header() const
    {
        return m_sentinel;
    }

    void insert_and_rebalance(bool insert_left, binary_node* p, binary_node& header)
    {
        this->insert_node_and_update_header(insert_left, p, header);
    }

    binary_node* rebalance_for_erase(binary_node& header)
    {
        this->replace_node_with_successor(header);
        return this;
    }

    void clone(const binary_node*)
    { }

    std::string to_string() const 
    {
        return "";
    }
};

}

