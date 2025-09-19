#pragma once

#include <memory>
#include <string>

namespace cpp::config::xml
{

template <typename T>
using global_allocator = std::allocator<T>;

using string = std::basic_string<char, std::char_traits<char>, global_allocator<char>>;

// Single linked list of attributes
struct attribute
{
    string name;
    string value;
    attribute* next = nullptr;
};

class element
{
    attribute* m_attr;
    string m_name;
    string m_value;

    element* m_child;
    element* m_next;
    element* m_prev;
    element* m_parent;
};

class document
{
    element* root = nullptr;
};

}
