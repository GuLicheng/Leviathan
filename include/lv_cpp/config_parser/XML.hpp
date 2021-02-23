// https://zh.wikipedia.org/wiki/XML
// https://www.runoob.com/xml/xml-examples.html

#ifndef __XML_HPP__
#define __XML_HPP__


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

    typedef struct _List
    {
        void** data;
        size_t size;
        size_t capacity;
    }list_t; 

    struct XML_tag
    {
        char* name;
        list_t attributes;
    };
    // <greeting>Hello, world!</greeting>
    struct XML_node
    {
        char* element; // Hello, world
        struct XML_tag* tag; // <greeting>
        struct XML_node* nested; // ...
    };
    
    



} // namespace leviathan::xml



#endif