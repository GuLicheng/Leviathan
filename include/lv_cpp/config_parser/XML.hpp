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

#define dist(x, y, msg) (std::cout << msg << (y - x) << std::endl)

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
        std::string body;
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
        const char* cur;
    public:

        XML_document() : root(nullptr), stack{} 
        {
        }

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
        bool parse_label(const char* start, const char* end);
        void parse_declearation(int offset, const char* start, const char* end);
        void parse_close(const char* start, const char* end);
        void parse_commit(const char* start, const char* end);
        void parse_text(const char* start, const char* end);
    };

    void XML_document::parse_text(const char* start, const char* end)
    {
        auto iter = std::find_if_not(start, end, ::isblank);
        if (iter == end) 
            return; // nothing
        if (*iter == '<')
        {
            // <a>  <b>... </b> </a>
            //    ^ ^
            //      iter
            xml_label* new_node = new xml_label();
            stack.back()->children.emplace_back(new_node);
            stack.emplace_back(new_node);
            cur = iter; 
            parse_label(iter, end);
        }
        else
        {
            console::write_line("text");
            //  <a>inner text </a>
            //     ^            ^
            //                ^
            auto body_end = std::find(iter, end, '<');
            stack.back()->body = std::string(start, body_end);
            cur = body_end;
            parse_close(cur, end);
        }

    }

    void XML_document::parse_commit(const char* start, const char* end)
    {
        // <!-- something here-->
        auto iter = std::find(start, end, '>');
        if (iter == end)
            assert(false);
        cur = iter + 1;
    }

    bool XML_document::read(const char* path)
    {
        if (root != nullptr)
            assert(false);
        // read file
        std::ifstream in{path, std::ios::in | std::ios::binary};
        
        // if file not exist, return false
        if (!in.is_open())
            return false;

        const std::string buffer{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
        std::cout << buffer << std::endl;
        const auto size = buffer.size();
        const auto end = buffer.data() + buffer.size();  // end of file
        auto iter = std::find_if_not(buffer.data(), end, ::isspace);  // the first char must be '<'
        // empty
        if (iter == end)
            return false;

        // must start with '<'
        if (*iter != '<')
            return false;
        cur = iter;
        root = new xml_label();
        root->name = "I am the root";
        stack.emplace_back(root);

        // std::cout << "Here";

        parse_text(cur, end);

        return true;    
    }

    bool XML_document::parse_label(const char* start, const char* end)
    {
        // std::string s {start, end};
        // console::write_line(s);
        while (cur != end)
        {
            const char ch = *(start + 1);
            if (ch == '?')
                parse_declearation(2, start, end);
            else if (::isalpha(ch))
                parse_declearation(1, start, end);
            else if (ch == '!')
                // parse commit
                parse_commit(cur, end);
            else if (ch == '/')
                parse_close(start, end);
            else
                return false;
            // error
        }
        return true;
    }

    void XML_document::parse_declearation(int offset, const char* start, const char* end)
    {
        // start is '<' and end is EOF

        auto right_angle_bracket = std::find(start, end, '>');
        // console::write_line(*right_angle_bracket);
        if (right_angle_bracket == end) 
            assert(false);


        // left <
        auto left_angle_bracket = start;
        // <name 
        //      ^
        //      iter
        auto iter = std::find_if(left_angle_bracket, right_angle_bracket, ::isspace);
        std::string label_name{left_angle_bracket + offset, iter};
        stack.back()->name = label_name;
        iter = std::find_if_not(iter, right_angle_bracket, ::isspace);

        if (iter == right_angle_bracket)
        {
            // not attr

            console::write_line_multi("msg is ", stack.back()->name);
            cur = iter + 1;
            parse_text(cur, end);
            return;
        }
        assert(::isalpha(*iter));
        // parse attr
    
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
            stack.back()->attributes.emplace_back(std::move(attr_name), std::move(attr_value));
            ch++;
            while (ch != right && !::isalpha(*ch)) ch++;
            left = ch;
        }

        // stack.emplace_back(node); // match close label
        cur = right_angle_bracket + 1;
        const auto ch = *(right_angle_bracket - 1);
        if (ch == '/' || ch == '?')
        {
            // just a label without body
            // exit(0);
            cur = std::find(cur, end, '<');

            //  console::write_line("Here");;
            // console::write_line_multi("cur is ", *cur);
            parse_label(cur, end);
        }
        else
        {
            // parse body and match close-label
            parse_text(cur, end);
        }

        std::cout << "OK" << std::endl;
    }

    void XML_document::parse_close(const char* start, const char* end)
    {
        assert(stack.size() > 0);
        //    </id>
        //    ^   ^
        // start  right_...
        auto right_angel_bracket = std::find(start, end, '>');
        const size_t length = right_angel_bracket - start - 2;
        const auto& back = stack.back()->name;
        // for (auto p = start + 2; p != right_angel_bracket; ++p) std::cout << *p;
        std::string match{start + 2, right_angel_bracket};
        std::cout << std::endl << "back is " << back << " and match is " << match << std::endl;
        if (length != back.size() || !std::equal(start + 2, right_angel_bracket, back.data()))
        {
            puts("All Node:");
            for (auto& node : stack) console::write_line(node->name);
            assert(false);
            // not match
        }
        else
        {
            stack.pop_back();
        }
        // stack.pop_back();
        cur = std::find(cur, end, '<'); // next_left_angle_bracket
        if (cur == end)
            return;  // over
        root->children.emplace_back(new xml_label());
        stack.emplace_back(root->children.back());
        parse_label(cur, end);
    }

} // namespace leviathan::xml



#endif