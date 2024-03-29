/**

 * !!! 我们的目的是能够正确读取红色警戒系列中的ini配置文件

 * 说明：ini配置文件读写

 * 1、支持;注释符号。// 非必须不支持 # 注释符号

 * 2、不支持带引号'或"成对匹配的字符串，否则无法正确读取相关词条，比如 name = stalin's fist

 * 3、如果无section，将读取不到任何信息，但section可以为空。

 * 4、支持十进制整数，浮点数以及Boolean的解析

 * 5、支持section、entry或=号前后带空格。

 * 6、按行读取，不接受换行。如何您有更高的要求，请使用json文件。

 * 7、区分section、key大小写。

 * 8、读取文件的时候不会对section和entry内容部分做任何检查， 但是可能会额外设置了一个接口会对读取后的结果进行检查，如何需要的话可以使用。

 * 9、每个key只有一个value，如果当前key已经存在且包含value那么将会覆盖之前的value

 * 10、由于不知道+=的含义，对此不做处理， 比如
            Line1: 19	=	THOR
            Line2: +=STHOR
 */

/*
    
    INI_handler::load(const char* file) : read ini config file
    INI_handler::load(const string& file) : read ini config file
    INI_handler::load(const string_view file) : read ini config file

    INI_handler::get_items(): return all sections and entries
    INI_handler::get_sections(): return all sections
    INI_handler::get_entries(): return all entries
    INI_handler::getint():  parser value to integer
    INI_handler::getfloat(): parser value to floating
    INI_handler::getboolean(): parser value to boolean
    INI_handler::getstring(): get value

    INI_handler::write(const char* file);
    INI_handler::write(const std::string& file);
    INI_handler::write(std::string_view file);
    
    follow method will be implemented by overloading operator[] such as map/unordered_map
    INI_handler::add_sections 
    INI_handler::add_key
    INI_handler::add_value
    examples:
        INI_handler reader;
        reader["section_name"]; // add a section
        reader["section_name"]["key_name"]; // add entry with key "key_name" and value ""
        reader["section_name"]["key_name"] = "value"; // add entry with key "key_name" and value "value"
*/

#ifndef __INI_HPP__
#define __INI_HPP__

#include "./base.hpp"

#include <unordered_map>
#include <fstream>
#include <list>
#include <string>
#include <string_view>
#include <optional>
#include <cctype>
#include <iostream>
#include <ranges>

namespace leviathan::ini
{

    using error_log = leviathan::parser::error_log;
    using entry = leviathan::parser::entry;

    // list of key-value
    struct section_node
    {
        section_node() = default;
        section_node(const section_node&) = default;
        section_node(section_node&&) noexcept = default;
        section_node& operator=(const section_node&) = default;
        section_node& operator=(section_node&&) noexcept = default;
        ~section_node() = default;

        std::list<entry> ls;
        std::string& operator[](const std::string& section_name)
        {
            auto iter = std::find_if(ls.begin(), ls.end(), [&](auto&& e)
            {
                return e.key() == section_name;
            });
            if (iter == ls.end())
                return ls.emplace_back(section_name, std::string("")).value();
            return iter->value();
        }
        std::string& operator[](std::string&& section_name)
        {
            auto iter = std::find_if(ls.begin(), ls.end(), [&](auto&& e)
            {
                return e.key() == section_name;
            });
            if (iter == ls.end())
                return ls.emplace_back(std::move(section_name), std::string("")).value();
            return iter->value();
        }
    };

    // overload ostream
    std::ostream& operator<<(std::ostream& os, const section_node& s)
    {
        std::cout << "The size of current section node is :" << s.ls.size() << std::endl;
        for (auto& e : s.ls)
        {
            std::cout << e << std::endl;
        }
        return os;
    }

    class INI_handler
    {
    public:
        INI_handler() : lines{0} 
        {    
        }

        INI_handler(const INI_handler&) = delete;
        INI_handler& operator=(const INI_handler&) = delete;
        ~INI_handler() = default;

        /**
         * @brief: load ini file
         * @param:
         *      file: the path of file
         * @return:
         *      true if successful load file and parse all items
         *      false otherwise
         */
        bool load(const char* file);
        bool load(const std::string& file)
        { 
            return load(file.c_str()); 
        }
        bool load(const std::string_view& file)
        {
            return load(file.data());
        }

        // erase all data within in this class
        // bool clear(section_node* node, entry* e);
        
        /**
         * whether the file is successfully loaded
         * @return:
         *      true if load return true, otherwise false
         */
        bool is_loaded() const noexcept
        { 
            return sections.size();
        }

        /**
         * get all sections from ini read
         * @return:
         *      view of sections
         */
        auto get_sections() const noexcept
        {
            return sections | ::std::views::keys;
        }

        /**
         * get all entries from ini read
         * @return:
         *      view of entries
         */
        auto get_entries() const noexcept
        {
            return sections | ::std::views::values;
        }

        /**
         * get all items from ini read
         * @return:
         *      view of items
         */
        auto get_items() const noexcept
        {
            return sections | ::std::views::all;
        }

        /**
         * parse a string to integer
         * @param:
         *      section_name: ...
         *      key_name: ...
         * @return:
         *      optional if both section_name and key_name exist and the value can be 
         *      convert to integer, otherwise nullpot
         */
        std::optional<int64_t> 
        getint(const std::string& section_name, const std::string& key_name) const;

        /**
         * parse a string to float
         * @param:
         *      section_name: ...
         *      key_name: ...
         * @return:
         *      optional if both section_name and key_name exist and the value can be 
         *      convert to float, otherwise nullpot
         */
        std::optional<double> 
        getfloat(const std::string& section_name, const std::string& key_name) const;

        /**
         * parse a string to boolean
         * @param:
         *      section_name: ...
         *      key_name: ...
         * @return:
         *      optional if both section_name and key_name exist and the value can be 
         *      convert to boolean(true or false, ignore case), otherwise nullpot
         */
        std::optional<bool>
        getboolean(const std::string& section_name, const std::string& key_name) const;

        /**
         * get value item by section and key
         * @param:
         *      section_name: ...
         *      key_name: ...
         * @return:
         *      optional if both section_name and key_name exist, otherwise nullopt
         */
        std::optional<std::string>
        getstring(const std::string& section_name, const std::string& key_name) const;

        /**
         * add or change a section into handler such as map/unordered_map
         * you can simply use reader[section_name][key_name] = value_name
         * for adding or changing 
         * @param:
         *      section_name: ...
         * @return:
         *      reference of section_node,if section_name not exist, 
         *      it will create a new section_node
         */
        section_node& operator[](const std::string& section_name)
        {
            return sections[section_name];
        }

        // for less than 15(or some other number) charactors, 
        // std::string may not allocate memory on heap
        // so I just put it here
        section_node& operator[](std::string&& section_name)
        {
            return sections[std::move(section_name)];
        }

        /**
         * write all items in handler into file
         * @param:
         *      file: the destiny file
         */
        void write(const char* file);

        void write(const std::string& file)
        {
            write(file.c_str());
        }

        void write(const std::string_view& file)
        {
            write(file.data());
        }

        // for debug
        void show()
        {
            for (auto& [key, value] : sections)
            {
                std::cout << key << "\n";
                std::cout << value << std::endl;
            }

            std::cout << "Here is log\n";
            for (auto&& l : log)
            {
                l.report();
            }
        }

    private:

        // remove all ; and blank
        std::string trim(const std::string& s) const noexcept;

        /** 
         * insert a section, if insert successfully, the node will pointer at section 
         * otherwise the log will memory this line
         * 
         * @param:
         *      node: the final address if insert successfully otherwise nullptr
         *      s: current string read from file
         *      line: the location of s in the file
         */
        void insert_section(section_node*& node, std::string s, int line);

    public:
        // I may change HashTable to LinkList some day
        std::unordered_map<std::string, section_node> sections;
        std::list<error_log> log; // save the line of error sections or entry
        int lines;
    }; //  end of class INI_handler


    bool INI_handler::load(const char* file) 
    {
        std::ifstream in{file, std::ios::in | std::ios::binary};

        // file not exist
        if (!in.is_open())
            return false;

        section_node* node = nullptr;
        std::string line;

        // prescan and get first section
        while (!in.eof())
        {
            std::getline(in, line);
            ++lines;
            auto str = trim(line);
            if (str.empty())
                continue;
            if (str[0] == '[')
            {
                insert_section(node, std::move(str), lines);
                if (node) break;
                else continue;
            }
        }

        // parse
        while (!in.eof())
        {
            std::getline(in, line);
            ++lines;
            auto str = trim(line);

            if (str.empty())
                continue;

            // handle section node
            
            if (str[0] == '[')
            {
                // parse sections
                insert_section(node, std::move(str), lines);
                continue;
            }
            // parse entry

            // the item in the left of '=' is key, right is value
            auto equal = str.find('=');

            // don't contain '='
            if (equal == str.npos)
            {
                // syntax error
                log.emplace_back(lines, std::move(str), "Not found =.");
                continue;
            }

            // start with '=' such as =foobar
            if (equal == 0)
            {
                log.emplace_back(lines, std::move(str), "Not found key.");
                continue;
            }

            // end with '=' such as foobar=
            if (equal == str.size() - 1)
            {
                log.emplace_back(lines, std::move(str), "Not found Value.");
                continue;
            }

            // trim key
            std::ranges::subrange left{str.begin(), str.begin() + equal};
            auto left_part = left 
                            | ::std::views::reverse
                            | ::std::views::drop_while(::isspace)
                            | ::std::views::reverse;
            auto key = std::string(left_part.begin(), left_part.end());

            // trim value
            std::ranges::subrange right{str.begin() + equal + 1, str.end()};
            auto right_part = right | ::std::views::drop_while(::isspace);
            auto value = std::string(right_part.begin(), right_part.end());
            
            // save entry
            node->ls.emplace_back(std::move(key), std::move(value));
        }
     
        return true;
    }

    void INI_handler::insert_section(section_node*& node, std::string s, int line)
    {
        // check whether the sections only contains one '[' and ']'
        // if not match, not change node
        if (s.find_first_of(']') != s.size() - 1 || s.find_last_of('[') != 0)
        {
            log.emplace_back(line, std::move(s), "more than one [ or ].");
            return;
        }
        node = &(sections.try_emplace(s.substr(1, s.size() - 2), section_node()).first->second);
    }

    std::string INI_handler::trim(const std::string& s) const noexcept
    {
        auto iter = std::find(s.begin(), s.end(), ';');
        std::ranges::subrange sub{s.cbegin(), iter};
        auto res_range = sub 
                        | ::std::views::drop_while(::isspace)
                        | ::std::views::reverse
                        | ::std::views::drop_while(::isspace)
                        | ::std::views::reverse;
        return {res_range.begin(), res_range.end()};
    }

    std::optional<int64_t> 
    INI_handler::getint(const std::string& section_name, const std::string& key_name) const  
    {
        auto value = getstring(section_name, key_name);
        if (!value.has_value())
            return {};
        try
        {
            auto res = std::stod(*value);
            return res;
        }
        catch(...)
        {
            return {};
        }
        return {}; // useless
    }

    std::optional<double> 
    INI_handler::getfloat(const std::string& section_name, const std::string& key_name) const  
    {
        auto value = getstring(section_name, key_name);
        if (!value.has_value())
            return {};
        try
        {
            auto res = std::stod(*value);
            return res;
        }
        catch(...)
        {
            return {};
        }
        return {}; // useless
    }

    std::optional<bool>
    INI_handler::getboolean(const std::string& section_name, const std::string& key_name) const
    {
        auto value = getstring(section_name, key_name);
        if (!value.has_value())
            return {};
    
        std::for_each(value->begin(), value->end(), [](char& c)
        {
            c = ::tolower(c);
        });
        if (value == "true")
            return true;
        if (value == "false")
            return false;
        return {};
    }

    std::optional<std::string>
    INI_handler::getstring(const std::string& section_name, const std::string& key_name) const
    {
        auto sec_iter = sections.find(section_name);
        if (sec_iter == sections.end())
            return {};
        auto value_iter = std::find_if(sec_iter->second.ls.begin(), sec_iter->second.ls.end(), [&](const auto& e)
        {
            return e.key() == key_name;
        });

        if (value_iter == sec_iter->second.ls.end())
            return {};
        return value_iter->second;
    }
    
    void INI_handler::write(const char* file) 
    {
        std::ofstream out{file};
        if (!out.is_open()) 
        {
            std::cout << "File Not Exist\n";
            return;
        }
        for (auto&& [section_name, entry_name] : sections)
        {
            out << '[' << section_name << ']' << '\n';
            for (auto&& [key_name, value_name] : entry_name.ls)
            {
                out << key_name << " = " << value_name << '\n';
            }
            out << '\n';
        }
        out.close();
    }

} // namespace leviathan::ini




#endif