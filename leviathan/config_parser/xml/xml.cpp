#include <iostream>
#include <format>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>
#include <leviathan/time/timer.hpp>
#include <ranges>
#include <ctime>
#include <leviathan/config_parser/xml/document.hpp>
#include <leviathan/print.hpp>

namespace xml = leviathan::config::xml;

/*
<bookstore>
    <book category="CHILDREN">
        <title>Harry Potter</title>
        <author>J K. Rowling</author>
        <year>2005</year>
        <price>29.99</price>
    </book>
    <book category="WEB">
        <title>Learning XML</title>
        <author>Erik T. Ray</author>
        <year>2003</year>
        <price>39.95</price>
    </book>
</bookstore>
*/

int main(int argc, char const *argv[])
{   
    xml::element* bookstore = new xml::element("bookstore", nullptr); 

    xml::element* book1 = new xml::element("book", bookstore);
    book1->m_attributes.emplace_back("category", "CHILDREN");

    xml::element* title1 = new xml::element("title", book1, "Harry Potter");
    xml::element* author1 = new xml::element("author", book1, "J K. Rowling");
    xml::element* year1 = new xml::element("year", book1, "2005");
    xml::element* price1 = new xml::element("price", book1, "29.99");

    book1->add_child(title1);
    book1->add_child(author1);
    book1->add_child(year1);
    book1->add_child(price1);

    xml::element* book2 = new xml::element("book", bookstore);
    book2->m_attributes.emplace_back("category", "WEB");
    
    xml::element* title2 = new xml::element("title", book2, "Learning XML");
    xml::element* author2 = new xml::element("author", book2, "Erik T. Ray");
    xml::element* year2 = new xml::element("year", book2, "2003");
    xml::element* price2 = new xml::element("price", book2, "39.95");

    book2->add_child(title2);
    book2->add_child(author2);
    book2->add_child(year2);
    book2->add_child(price2);

    bookstore->add_child(book1);
    bookstore->add_child(book2);

    xml::document d(xml::declaration{"1.0", "UTF-8"}, bookstore);

    d.show();

    delete bookstore;

    Console::WriteLine("OK");

    return 0;
}
