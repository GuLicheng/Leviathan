#pragma once

#include "element.hpp"

namespace cpp::config::xml
{
   
// <?xml version="1.0" encoding="UTF-8"?>
struct declaration
{
    string m_version;
    string m_encoding;
};

class document
{
public:

    document(declaration decl, element* root) : m_declaration(decl), m_root(root) 
    { }

    declaration m_declaration;
    element* m_root;

    void show() const
    {
        std::cout << std::format("<?xml version='{}' encoding='{}'?>\n", 
            m_declaration.m_version, m_declaration.m_encoding);

        show_element_tree(m_root);
    }

};

} // namespace cpp::config::xml

