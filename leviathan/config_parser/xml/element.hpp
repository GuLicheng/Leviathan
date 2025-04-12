#pragma once

#include <memory>
#include <format>
#include <leviathan/variable.hpp>

namespace cpp::config::xml
{
    
template <typename T>
using global_allocator = std::allocator<T>;

template <typename T, typename... Args>
T* allocate_and_construct(Args&&... args)
{
    auto ptr = global_allocator<T>().allocate(1);
    std::construct_at(ptr, (Args&&)args...);
    return ptr;
}

template <typename T>
void destroy_and_deallocate(T* ptr)
{
    std::destroy_at(ptr);
    global_allocator<T>().deallocate(ptr, 1);
}

using string = std::basic_string<char, std::char_traits<char>, global_allocator<char>>;

template <size_t N>
struct as_ptr_if_large_than
{
    template <typename U>
    using type = std::conditional_t<(sizeof(U) > N), U*, U>;

    template <typename T>
    static constexpr auto from_value(T t) 
    {
        if constexpr (sizeof(T) > N)
        {
            return allocate_and_construct<T>(std::move(t));
        }
        else
        {
            return t;
        }
    }

    template <typename T>
    static constexpr auto to_address(T* t) 
    {
        if constexpr (!std::is_pointer_v<T>)
        {
            return t;
        }
        else
        {
            return std::to_address(*t);
        }
    }
};

class element;
using attribute = std::pair<string, string>;
using attribute_list = std::vector<attribute, global_allocator<attribute>>;
using element_list = std::vector<element*, global_allocator<element*>>;

// <nobody a="b" c="d"/>
// <note xmlns="http://www.w3.org/1999/xhtml" xmlns:leviathan="http://www.leviathan.com">
//     <to>Tove</to>
//     <from>Jani</from>
//     <heading>Reminder</heading>
//     <body>Don't forget me this weekend!</body>
//     <nobody a="b" c="d"/>
//     <>
// </note>

using body = cpp::variable<as_ptr_if_large_than<16>, std::monostate, string, element_list>;

class element
{
public:

    element(string label, element* parent)
        : m_tag(std::move(label)), m_parent(parent)
    { }

    element(string label, element* parent, string text)
        : m_tag(std::move(label)), m_parent(parent), m_children_or_text(std::move(text))
    { }

    element(const element&) = delete;
    element& operator=(const element&) = delete;

    string m_tag;  // book

    element* m_parent;

    attribute_list* m_attributes = nullptr;  // category="cooking"
    
    body m_children_or_text;

    bool has_children() const
    {
        return this->m_children_or_text.index() == 2;
    }

    bool is_leaf() const 
    {
        return this->m_children_or_text.index() == 1;
    }

    bool has_attributes() const
    {
        return m_attributes != nullptr;
    }

    void add_attribute(string name, string value)
    {
        if (this->m_attributes == nullptr)
        {
            this->m_attributes = allocate_and_construct<attribute_list>();
        }

        this->m_attributes->emplace_back(std::move(name), std::move(value));
    }

    void add_child(element* child)
    {
        if (this->m_children_or_text.index() == 1)
        {
            throw std::runtime_error("Cannot add child to leaf node.");
        }

        if (this->m_children_or_text.index() == 0)
        {
            this->m_children_or_text = element_list();
        }

        this->m_children_or_text.as<element_list>().push_back(child);
    }

    void remove_children()
    {
        if (this->m_children_or_text.is<element_list>())
        {
            auto& ls = this->m_children_or_text.as<element_list>();
            
            for (auto& child : ls)
            {
                child->remove_children();
                std::cout << "Deleting " << child->m_tag << std::endl;
                delete child;
            }

            ls.clear();
        }
    }

    ~element()
    {
        remove_children();

        if (m_attributes)
        {
            destroy_and_deallocate(m_attributes);
        }
    }
};

template <typename... Args>
element* make_element(Args&&... args)
{
    throw std::runtime_error("Not implemented");
    // using Alloc = global_allocator<element>;
    // Alloc alloc;
    // return std::allocator_traits<Alloc>::allocate(alloc, (Args&&)args...);
}

/*
<?xml version="1.0" encoding="UTF-8"?>
<note xmlns="http://www.w3.org/1999/xhtml" xmlns:leviathan="http://www.leviathan.com">
    <to>Tove</to>
    <from>Jani</from>
</note>
*/
inline void show_element_tree(element* root, int depth = 0)
{
    if (!root)
    {
        return;
    }

    auto blank = [](int d) 
    {
        for (int i = 0; i < d; ++i)
        {
            std::cout << "\t";
        }
    };

    blank(depth);

    std::cout << std::format("<{}", root->m_tag);

    if (root->has_attributes())
    {
        for (const auto& [name, value] : *(root->m_attributes))
        {
            std::cout << std::format(" {}='{}'", name, value);
        }
    }

    std::cout << '>';

    if (root->is_leaf())
    {
        std::cout << std::format("{}</{}>\n", 
            root->m_children_or_text.as<string>(),
            root->m_tag);
    }
    else
    {
        std::cout << '\n';

        for (auto child : root->m_children_or_text.as<element_list>())
        {
            show_element_tree(child, depth + 1);
        }

        blank(depth);
        std::cout << std::format("</{}>\n", root->m_tag);
    }
}

} // namespace cpp::config::xml

