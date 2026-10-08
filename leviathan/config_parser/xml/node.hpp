/*
    Node

        - Element
        - Text
        - Comment
        - ProcessingInstruction
        - CDataSection
        - XmlEntityReference -> Unfold to a new node
        - ProcessingInstruction
        - Document
*/

#pragma once

#include <leviathan/variable.hpp>
#include <leviathan/config_parser/common.hpp>
#include <leviathan/extc++/format.hpp>

#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

namespace cpp::config::xml
{

template <typename T>
using global_allocator = std::allocator<T>;

using string = std::basic_string<char, std::char_traits<char>, global_allocator<char>>;

template <typename T>
using vector = std::vector<T, global_allocator<T>>;

class node;
class element;
class processing_instruction;
class document;

using attribute = std::pair<string, string>;

// We wrap text, comment, and cdata_section in structs. For 
// std::variable we need to differentiate these types from plain strings.
struct text { string value; };
struct comment { string value; };
struct cdata_section { string value; };

class element
{
    string name;
    vector<attribute> m_attributes;
    vector<node*> m_children;

public:

    element(const string& name) : name(name) { }

    bool add_attribute(string key, string value) 
    {
        // Attribute already exists
        if (std::ranges::contains(m_attributes, key, attribute::first))
        {
            return false; 
        }
        m_attributes.emplace_back(std::move(key), std::move(value));
        return true;
    }

};

template <typename T>
struct deleter
{
    static void constexpr operator()(T* p) 
    { 
        std::destroy_at(p);
        global_allocator<T> alloc;
        std::allocator_traits<global_allocator<T>>::deallocate(alloc, p, 1);
    };
};

template <size_t N>
struct as_unique_ptr_if_large_than
{
    template <typename U>
    using type = std::conditional_t<(sizeof(U) > N), std::unique_ptr<U, deleter<U>>, U>;

    template <typename T>
    static constexpr auto from_value(T t) 
    {
        if constexpr (sizeof(T) > N)
        {
            global_allocator<T> alloc;
            auto ptr = std::allocator_traits<global_allocator<T>>::allocate(alloc, 1);
            std::construct_at(ptr, std::move(t));            
            return std::unique_ptr<T, deleter<T>>(ptr, deleter<T>());
        }
        else
        {
            return t;
        }
    }

    template <typename T>
    static constexpr auto to_address(T* t) 
    {
        if constexpr (!refl::instance_of_template(^^T, ^^std::unique_ptr))
        {
            return t;
        }
        else
        {
            return std::to_address(*t);
        }
    }
};

using node_base = variable<
    as_unique_ptr_if_large_than<32>,  // We make string on stack
    element,
    comment,
    processing_instruction,
    cdata_section,
    document
>;

class node : public node_base
{

public:

    using base = node_base;
    using base::base;
    using base::operator=;

    template <typename T>
    node(T x) : base(cpp::cast<node>(std::move(x)))
    { }
};

template <typename Object, typename... Args>
node make_node(Args&&... args)
{
    return Object((Args&&) args...);
}


} // namespace cpp::config::xml
