// https://zh.wikipedia.org/wiki/XML
// https://www.runoob.com/xml/xml-examples.html

#ifndef __XML_HPP__
#define __XML_HPP__

#include "./base.hpp"

#include <cstdint>
#include <string>
#include <fstream>
#include <utility>
#include <vector>
#include <memory>


/*
      <!--联系信息
			ElementNo
			No				电话号码
			City			城市三字代码
			PsgID			旅客编号
			Text			文本信息
	  -->

第一行必须是以下内容
<?xml version="1.0" encoding="UTF-8"?>
<!--  Edited by XMLSpy®  -->
<note>
    可以包含多个内嵌标签
    <to>Tove</to>
    <from>Jani</from>
    <heading>Reminder</heading>
    <body>Don't forget me this weekend!</body>
</note>

*/
namespace leviathan::xml
{

    /*
        start-tag <project version="4">
        -> start with '<' and end with '>'
        end-tag </project>
        -> start with '</' and end with '>'
        empty-element-tag <output url="file://$PROJECT_DIR$/out" />
        -> start with '<' and end with '/>'
    */

    using attribute_entry = leviathan::parser::entry;
    using error_log = leviathan::parser::error_log;

    struct xml_label
    {
        std::string name;
        std::vector<attribute_entry> attributes;
        std::vector<xml_label*> children;
    };

    constexpr inline const char* space = " \t\n";
    
    class XML_document
    {
        xml_label* root;
    public:

        bool read(const char* path);
    };

    bool XML_document::read(const char* path)
    {
        // read file
        std::ifstream in{path, std::ios::in | std::ios::binary};
        
        // if file not exist, return false
        if (!in.is_open())
            return false;

        std::string buffer;
        buffer.resize(256);

        
    }

} // namespace leviathan::xml



#endif