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
#include <iterator>
#include <algorithm>

#include <lv_cpp/io/console.hpp>
#include <assert.h>

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

    std::ostream& operator<<(std::ostream& os, const xml_label& label)
    {
        std::cout << label.name << std::endl;
        for (auto& entry : label.attributes)
            std::cout << entry << std::endl;
    }

    constexpr inline const char* space = " \t\n";
    
    class XML_document
    {
        xml_label* root;
        std::vector<xml_label*> stack; // check pair label
    public:

        bool read(const char* path);


        void show()
        {
            if (!root)
            {
                std::cout << *root << std::endl;
            }
        }

    private:
        // <name sdasd>
        bool parse_label(xml_label* node, const char* start, const char* end);
        bool parse_declearation(int offset, xml_label* node, const char* start, const char* end);

    
    };

    bool XML_document::read(const char* path)
    {
        // read file
        std::ifstream in{path, std::ios::in | std::ios::binary};
        
        // if file not exist, return false
        if (!in.is_open())
            return false;

        const std::string buffer{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};;
        std::cout << buffer << std::endl;
        const auto size = buffer.size();
        const auto end = buffer.data() + buffer.size();
        auto iter = std::find_if_not(buffer.data(), end, ::isspace);
        // empty
        if (iter == end)
            return false;

        // must start with '<'
        if (*iter != '<')
            return false;

        xml_label* node = nullptr;

        parse_label(node, iter, end);

        return true;    
    }

    bool XML_document::parse_label(xml_label* node, const char* start, const char* end)
    {
        const char ch = *(start + 1);
        auto right_angle_bracket = std::find(start, end, '>');

        if (right_angle_bracket == end)
            return false;

        if (ch == '?')
            parse_declearation(2, node, start, right_angle_bracket);
        else if (::isalpha(ch))
            // parse identity
            return false; 
        else if (ch == '!')
            // parse commit
            return false;
        else if (ch == '/')
            // parse end
            return false;
        else 
            return false;
            // error
    }

    bool XML_document::parse_declearation(int offset, xml_label* node, const char* left_angle_bracket, const char* right_angle_bracket)
    {
        auto iter = std::find_if(left_angle_bracket, right_angle_bracket, ::isspace);
        std::string label_name{left_angle_bracket + offset, iter};
        iter = std::find_if_not(iter, right_angle_bracket, ::isspace);
        if (iter == right_angle_bracket || !::isalpha(*iter))
        {
            // <?xml 0id = >
            return false;
        }
        const char* left = iter;
        const char* right = right_angle_bracket;
        while (*right != '"') right--;
        right++;
        // console::write_line_multi(*left, *right, "over");
        for (auto ch = left + 1; ch != right;)
        {
            // parse name
            while (::isalpha(*ch)) ++ch;
            std::string attr_name(left, ch);

            // match =
            while (::isspace(*ch) || *ch == '=') ch++;
            // parse value
            assert(*ch == '"');
            left = ++ch;
            while (*ch != '"') ch++;
            std::string attr_value{left, ch};

            // std::cout << attr_name << " ; " << attr_value << std::endl;            
            console::write_line_multi("(", label_name, attr_name, attr_value, ")");
            // node->attributes.emplace_back(std::move(attr_name), std::move(attr_value));
            ch++;
            while (ch != right && !::isalpha(*ch)) ch++;
            left = ch;
        }
        std::cout << "OK" << std::endl;
        return true;
    }

} // namespace leviathan::xml



#endif